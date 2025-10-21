#pragma once

#include "../common.h"

namespace raytracer {
namespace core {

class Ray {
public:
    Ray() = default;
    Ray(const Point3& origin, const Vec3& direction) 
        : origin_(origin), direction_(direction) {}

    Point3 origin() const { return origin_; }
    Vec3 direction() const { return direction_; }
    
    Point3 at(float t) const {
        return origin_ + t * direction_;
    }

private:
    Point3 origin_;
    Vec3 direction_;
};

} // namespace core
} // namespace raytracer
