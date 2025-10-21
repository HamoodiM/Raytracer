#pragma once

#include "../geometry/primitive.h"
#include "aabb.h"
#include <memory>
#include <vector>

namespace raytracer {
namespace acceleration {

class BVHNode : public geometry::Primitive {
public:
    BVHNode() = default;
    BVHNode(const std::vector<std::shared_ptr<geometry::Primitive>>& src_objects, 
            size_t start, size_t end);

    bool hit(const core::Ray& ray, float t_min, float t_max, geometry::HitRecord& rec) const override;

private:
    std::shared_ptr<geometry::Primitive> left_;
    std::shared_ptr<geometry::Primitive> right_;
    AABB box_;
};

// Helper functions for sorting
bool box_compare(const std::shared_ptr<geometry::Primitive> a, 
                 const std::shared_ptr<geometry::Primitive> b, int axis);
bool box_x_compare(const std::shared_ptr<geometry::Primitive> a, 
                   const std::shared_ptr<geometry::Primitive> b);
bool box_y_compare(const std::shared_ptr<geometry::Primitive> a, 
                   const std::shared_ptr<geometry::Primitive> b);
bool box_z_compare(const std::shared_ptr<geometry::Primitive> a, 
                   const std::shared_ptr<geometry::Primitive> b);

} // namespace acceleration
} // namespace raytracer
