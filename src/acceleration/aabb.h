#pragma once

#include "../common.h"
#include "../core/ray.h"
#include <limits>

namespace raytracer {
namespace acceleration {

class AABB {
public:
    // Default box is empty (min = +inf, max = -inf), so expand() works from it.
    AABB()
        : minimum_(std::numeric_limits<float>::infinity()),
          maximum_(-std::numeric_limits<float>::infinity()) {}
    AABB(const Point3& a, const Point3& b) : minimum_(a), maximum_(b) {}

    Point3 min() const { return minimum_; }
    Point3 max() const { return maximum_; }

    bool hit(const core::Ray& ray, float t_min, float t_max) const;

    void expand(const Point3& p) {
        minimum_ = glm::min(minimum_, p);
        maximum_ = glm::max(maximum_, p);
    }
    void expand(const AABB& b) {
        minimum_ = glm::min(minimum_, b.minimum_);
        maximum_ = glm::max(maximum_, b.maximum_);
    }
    bool empty() const {
        return minimum_.x > maximum_.x || minimum_.y > maximum_.y || minimum_.z > maximum_.z;
    }
    Vec3 extent() const { return empty() ? Vec3(0.0f) : maximum_ - minimum_; }
    Point3 centroid() const { return 0.5f * (minimum_ + maximum_); }
    float surface_area() const {
        Vec3 e = extent();
        return 2.0f * (e.x * e.y + e.y * e.z + e.z * e.x);
    }
    int longest_axis() const {
        Vec3 e = extent();
        return (e.x >= e.y && e.x >= e.z) ? 0 : (e.y >= e.z ? 1 : 2);
    }

private:
    Point3 minimum_;
    Point3 maximum_;
};

AABB surrounding_box(const AABB& box0, const AABB& box1);

} // namespace acceleration
} // namespace raytracer
