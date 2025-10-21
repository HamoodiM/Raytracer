#include "bvh.h"
#include "../common.h"
#include "../core/utils.h"
#include <algorithm>

namespace raytracer {
namespace acceleration {

BVHNode::BVHNode(const std::vector<std::shared_ptr<geometry::Primitive>>& src_objects, 
                 size_t start, size_t end) {
    auto objects = src_objects; // Create a modifiable array of the source scene objects

    int axis = core::random_int(0, 2);
    auto comparator = (axis == 0) ? box_x_compare
                    : (axis == 1) ? box_y_compare
                                  : box_z_compare;

    size_t object_span = end - start;

    if (object_span == 1) {
        left_ = right_ = objects[start];
    } else if (object_span == 2) {
        if (comparator(objects[start], objects[start+1])) {
            left_ = objects[start];
            right_ = objects[start+1];
        } else {
            left_ = objects[start+1];
            right_ = objects[start];
        }
    } else {
        std::sort(objects.begin() + start, objects.begin() + end, comparator);

        auto mid = start + object_span/2;
        left_ = std::make_shared<BVHNode>(objects, start, mid);
        right_ = std::make_shared<BVHNode>(objects, mid, end);
    }

    // TODO: Implement bounding box calculation
    // box_ = surrounding_box(left_->bounding_box(), right_->bounding_box());
}

bool BVHNode::hit(const core::Ray& ray, float t_min, float t_max, geometry::HitRecord& rec) const {
    // TODO: Implement BVH traversal
    return false;
}

// Helper functions for sorting
bool box_compare(const std::shared_ptr<geometry::Primitive> a, 
                 const std::shared_ptr<geometry::Primitive> b, int axis) {
    // TODO: Implement box comparison
    return false;
}

bool box_x_compare(const std::shared_ptr<geometry::Primitive> a, 
                   const std::shared_ptr<geometry::Primitive> b) {
    return box_compare(a, b, 0);
}

bool box_y_compare(const std::shared_ptr<geometry::Primitive> a, 
                   const std::shared_ptr<geometry::Primitive> b) {
    return box_compare(a, b, 1);
}

bool box_z_compare(const std::shared_ptr<geometry::Primitive> a, 
                   const std::shared_ptr<geometry::Primitive> b) {
    return box_compare(a, b, 2);
}

} // namespace acceleration
} // namespace raytracer
