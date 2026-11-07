/**
 * @file bvh.h
 * @brief Bounding volume hierarchy over scene primitives
 *
 * Built top-down with a binned surface area heuristic (SAH) split, or a
 * simple spatial-midpoint split for comparison. Nodes are stored flat in
 * one array (children of an interior node are adjacent), and traversal is
 * iterative with a small stack, visiting the nearer child first.
 */

#pragma once

#include "../geometry/primitive.h"
#include "aabb.h"
#include <cstdint>
#include <memory>
#include <vector>

namespace raytracer {
namespace acceleration {

class BVH {
public:
    enum class SplitMethod { SAH, Middle };

    struct Stats {
        size_t primitives = 0;
        size_t nodes = 0;
        size_t leaves = 0;
        size_t max_depth = 0;
        size_t max_leaf_size = 0;
    };

    BVH() = default;

    /**
     * @brief Builds the hierarchy. Every primitive must have a bounding box.
     */
    void build(const std::vector<std::shared_ptr<geometry::Primitive>>& primitives,
               SplitMethod method = SplitMethod::SAH);

    /**
     * @brief Closest hit in (t_min, t_max), same contract as Primitive::hit
     */
    bool hit(const core::Ray& ray, float t_min, float t_max, geometry::HitRecord& rec) const;

    bool empty() const { return nodes_.empty(); }
    const Stats& stats() const { return stats_; }

private:
    struct Node {
        Point3 bmin;
        uint32_t first;    // leaf: first primitive index; interior: left child index (right = first + 1)
        Point3 bmax;
        uint16_t count;    // number of primitives; 0 for an interior node
        uint16_t axis;     // split axis of an interior node
    };

    struct PrimInfo {
        AABB box;
        Point3 centroid;
        uint32_t index;
    };

    static constexpr int kBins = 16;
    static constexpr int kMaxLeafSize = 4;
    static constexpr int kStackSize = 64;

    void subdivide(uint32_t node_index, std::vector<PrimInfo>& info, uint32_t begin, uint32_t end,
                   size_t depth, SplitMethod method);

    std::vector<Node> nodes_;
    std::vector<std::shared_ptr<geometry::Primitive>> owned_;  // keeps primitives alive
    std::vector<const geometry::Primitive*> prims_;            // leaf order
    Stats stats_;
};

} // namespace acceleration
} // namespace raytracer
