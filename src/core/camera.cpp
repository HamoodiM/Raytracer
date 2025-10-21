#include "camera.h"
#include "../common.h"
#include "utils.h"
#include <glm/gtc/constants.hpp>

namespace raytracer {
namespace core {

Camera::Camera(const Point3& lookfrom, const Point3& lookat, const Vec3& vup,
               float vfov, float aspect_ratio, float aperture, float focus_dist) {
    auto theta = glm::radians(vfov);
    auto h = tan(theta / 2);
    auto viewport_height = 2.0f * h;
    auto viewport_width = aspect_ratio * viewport_height;

    w_ = glm::normalize(lookfrom - lookat);
    u_ = glm::normalize(glm::cross(vup, w_));
    v_ = glm::cross(w_, u_);

    origin_ = lookfrom;
    horizontal_ = focus_dist * viewport_width * u_;
    vertical_ = focus_dist * viewport_height * v_;
    lower_left_corner_ = origin_ - horizontal_ / 2.0f - vertical_ / 2.0f - focus_dist * w_;

    lens_radius_ = aperture / 2.0f;
}

Ray Camera::get_ray(float s, float t) const {
    Vec3 rd = lens_radius_ * random_in_unit_disk();
    Vec3 offset = u_ * rd.x + v_ * rd.y;

    return Ray(origin_ + offset,
               lower_left_corner_ + s * horizontal_ + t * vertical_ - origin_ - offset);
}


} // namespace core
} // namespace raytracer
