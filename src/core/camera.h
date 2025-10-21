#pragma once

#include "ray.h"
#include <glm/glm.hpp>

namespace raytracer {
namespace core {

class Camera {
public:
    Camera() = default;
    Camera(const Point3& lookfrom, const Point3& lookat, const Vec3& vup,
           float vfov, float aspect_ratio, float aperture = 0.0f, float focus_dist = 1.0f);

    Ray get_ray(float u, float v) const;

private:
    Point3 origin_;
    Point3 lower_left_corner_;
    Vec3 horizontal_;
    Vec3 vertical_;
    Vec3 u_, v_, w_;
    float lens_radius_;
};

} // namespace core
} // namespace raytracer
