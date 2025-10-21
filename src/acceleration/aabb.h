#pragma once

#include "../common.h"
#include "../core/ray.h"

namespace raytracer {
namespace acceleration {

class AABB {
public:
    AABB() = default;
    AABB(const Point3& a, const Point3& b) : minimum_(a), maximum_(b) {}

    Point3 min() const { return minimum_; }
    Point3 max() const { return maximum_; }

    bool hit(const core::Ray& ray, float t_min, float t_max) const;

private:
    Point3 minimum_;
    Point3 maximum_;
};

AABB surrounding_box(const AABB& box0, const AABB& box1);

} // namespace acceleration
} // namespace raytracer
