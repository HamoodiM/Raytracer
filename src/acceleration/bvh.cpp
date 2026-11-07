/**
 * @file bvh.cpp
 * @brief Binned-SAH bounding volume hierarchy
 */

#include "bvh.h"
#include "../common.h"
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace raytracer {
namespace acceleration {

namespace {

// Ray-box slab test against the current closest hit distance.
inline bool slab_hit(const Point3& bmin, const Point3& bmax, const Point3& origin,
                     const Vec3& inv_dir, float t_min, float t_max) {
    Vec3 t0 = (bmin - origin) * inv_dir;
    Vec3 t1 = (bmax - origin) * inv_dir;
    Vec3 tsmall = glm::min(t0, t1);
    Vec3 tbig = glm::max(t0, t1);
    float enter = glm::max(glm::max(tsmall.x, tsmall.y), glm::max(tsmall.z, t_min));
    float exit = glm::min(glm::min(tbig.x, tbig.y), glm::min(tbig.z, t_max));
    return enter <= exit;
}

constexpr size_t kMaxBuildDepth = 60;  // keeps traversal within the fixed-size stack

} // namespace

void BVH::build(const std::vector<std::shared_ptr<geometry::Primitive>>& primitives,
                SplitMethod method) {
    nodes_.clear();
    prims_.clear();
    owned_ = primitives;
    stats_ = Stats();
    stats_.primitives = primitives.size();
    if (primitives.empty()) return;

    std::vector<PrimInfo> info(primitives.size());
    for (size_t i = 0; i < primitives.size(); ++i) {
        AABB box;
        if (!primitives[i]->bounding_box(box))
            throw std::runtime_error("BVH::build: primitive without a bounding box");
        info[i].box = box;
        info[i].centroid = box.centroid();
        info[i].index = static_cast<uint32_t>(i);
    }

    nodes_.reserve(2 * primitives.size());
    nodes_.push_back(Node());
    subdivide(0, info, 0, static_cast<uint32_t>(info.size()), 0, method);

    prims_.reserve(info.size());
    for (const auto& pi : info) prims_.push_back(owned_[pi.index].get());
    stats_.nodes = nodes_.size();
}

void BVH::subdivide(uint32_t node_index, std::vector<PrimInfo>& info, uint32_t begin,
                    uint32_t end, size_t depth, SplitMethod method) {
    AABB bounds, centroid_bounds;
    for (uint32_t i = begin; i < end; ++i) {
        bounds.expand(info[i].box);
        centroid_bounds.expand(info[i].centroid);
    }
    nodes_[node_index].bmin = bounds.min();
    nodes_[node_index].bmax = bounds.max();
    stats_.max_depth = std::max(stats_.max_depth, depth);

    const uint32_t n = end - begin;
    auto make_leaf = [&]() {
        nodes_[node_index].first = begin;
        nodes_[node_index].count = static_cast<uint16_t>(n);
        nodes_[node_index].axis = 0;
        stats_.leaves++;
        stats_.max_leaf_size = std::max(stats_.max_leaf_size, static_cast<size_t>(n));
    };

    if (n <= 1 || (depth >= kMaxBuildDepth && n <= 0xFFFF)) {
        make_leaf();
        return;
    }

    int axis = centroid_bounds.longest_axis();
    const Vec3 cext = centroid_bounds.extent();
    uint32_t mid = begin;

    if (cext[axis] <= 0.0f) {
        // All centroids coincide: no spatial split can separate them.
        if (n <= kMaxLeafSize) { make_leaf(); return; }
        mid = begin + n / 2;  // split by count below
    } else if (method == SplitMethod::Middle) {
        if (n <= 2) { make_leaf(); return; }
        const float split = centroid_bounds.centroid()[axis];
        auto it = std::partition(info.begin() + begin, info.begin() + end,
                                 [&](const PrimInfo& p) { return p.centroid[axis] < split; });
        mid = static_cast<uint32_t>(it - info.begin());
    } else {
        // Binned SAH: for each axis, drop the centroids into kBins bins, then
        // try the kBins - 1 bin boundaries as split planes and estimate
        //   cost = C_trav + (A_left * N_left + A_right * N_right) / A_node * C_isect
        // with C_trav = C_isect = 1. A_child / A_node is the probability that a
        // random ray hitting the node also hits the child's box.
        float best_cost = std::numeric_limits<float>::infinity();
        int best_axis = -1, best_split = -1;
        const float node_area = bounds.surface_area();

        for (int a = 0; a < 3; ++a) {
            if (cext[a] <= 0.0f) continue;
            AABB bin_box[kBins];
            int bin_count[kBins] = {0};
            const float scale = kBins / cext[a];
            const float cmin = centroid_bounds.min()[a];
            for (uint32_t i = begin; i < end; ++i) {
                int b = std::min(kBins - 1, static_cast<int>((info[i].centroid[a] - cmin) * scale));
                bin_count[b]++;
                bin_box[b].expand(info[i].box);
            }
            // Prefix sweep from the left and suffix sweep from the right
            float left_area[kBins - 1], right_area[kBins - 1];
            int left_count[kBins - 1], right_count[kBins - 1];
            AABB lb, rb;
            int lc = 0, rc = 0;
            for (int i = 0; i < kBins - 1; ++i) {
                lc += bin_count[i];
                lb.expand(bin_box[i]);
                left_count[i] = lc;
                left_area[i] = lc ? lb.surface_area() : 0.0f;

                rc += bin_count[kBins - 1 - i];
                rb.expand(bin_box[kBins - 1 - i]);
                right_count[kBins - 2 - i] = rc;
                right_area[kBins - 2 - i] = rc ? rb.surface_area() : 0.0f;
            }
            for (int i = 0; i < kBins - 1; ++i) {
                if (left_count[i] == 0 || right_count[i] == 0) continue;
                float cost = 1.0f + (left_area[i] * left_count[i] + right_area[i] * right_count[i]) /
                                        node_area;
                if (cost < best_cost) {
                    best_cost = cost;
                    best_axis = a;
                    best_split = i;
                }
            }
        }

        // A leaf costs N primitive tests; keep small nodes as leaves when
        // splitting would not be cheaper.
        if (best_axis < 0 || (best_cost >= static_cast<float>(n) && n <= kMaxLeafSize)) {
            make_leaf();
            return;
        }
        axis = best_axis;
        const float scale = kBins / cext[axis];
        const float cmin = centroid_bounds.min()[axis];
        auto it = std::partition(info.begin() + begin, info.begin() + end, [&](const PrimInfo& p) {
            int b = std::min(kBins - 1, static_cast<int>((p.centroid[axis] - cmin) * scale));
            return b <= best_split;
        });
        mid = static_cast<uint32_t>(it - info.begin());
    }

    if (mid == begin || mid == end) {
        // Degenerate partition: fall back to a median split on the axis.
        mid = begin + n / 2;
        std::nth_element(info.begin() + begin, info.begin() + mid, info.begin() + end,
                         [&](const PrimInfo& x, const PrimInfo& y) {
                             return x.centroid[axis] < y.centroid[axis];
                         });
    }

    const uint32_t left = static_cast<uint32_t>(nodes_.size());
    nodes_.push_back(Node());
    nodes_.push_back(Node());
    nodes_[node_index].first = left;
    nodes_[node_index].count = 0;
    nodes_[node_index].axis = static_cast<uint16_t>(axis);
    subdivide(left, info, begin, mid, depth + 1, method);
    subdivide(left + 1, info, mid, end, depth + 1, method);
}

bool BVH::hit(const core::Ray& ray, float t_min, float t_max, geometry::HitRecord& rec) const {
    if (nodes_.empty()) return false;

    const Point3 origin = ray.origin();
    const Vec3 dir = ray.direction();
    const Vec3 inv_dir = 1.0f / dir;  // +-inf for zero components is fine for the slab test
    const bool dir_neg[3] = {dir.x < 0.0f, dir.y < 0.0f, dir.z < 0.0f};

    uint32_t stack[kStackSize];
    int sp = 0;
    stack[sp++] = 0;

    bool hit_anything = false;
    float closest = t_max;
    geometry::HitRecord temp;

    while (sp > 0) {
        const Node& node = nodes_[stack[--sp]];
        if (!slab_hit(node.bmin, node.bmax, origin, inv_dir, t_min, closest)) continue;

        if (node.count > 0) {
            for (uint32_t i = 0; i < node.count; ++i) {
                if (prims_[node.first + i]->hit(ray, t_min, closest, temp)) {
                    hit_anything = true;
                    closest = temp.t;
                    rec = temp;
                }
            }
        } else {
            // Push the far child first so the near child is popped next; a
            // near hit then shrinks 'closest' and lets the far box be culled.
            uint32_t near_child = node.first, far_child = node.first + 1;
            if (dir_neg[node.axis]) std::swap(near_child, far_child);
            stack[sp++] = far_child;
            stack[sp++] = near_child;
        }
    }

    return hit_anything;
}

} // namespace acceleration
} // namespace raytracer
