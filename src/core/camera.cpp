#include "camera.h"
#include "../common.h"
#include "utils.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace raytracer {
namespace core {

Camera::Camera(const Point3& lookfrom, const Point3& lookat, const Vec3& vup,
               float vfov, float aspect_ratio, float aperture, float focus_dist) 
    : origin_(lookfrom), lookat_(lookat), vup_(vup), vfov_(vfov), 
      aspect_ratio_(aspect_ratio), aperture_(aperture), focus_dist_(focus_dist) {
    rebuild();
}

Ray Camera::get_ray(float s, float t) const {
    Vec3 rd = lens_radius_ * random_in_unit_disk();
    Vec3 offset = u_ * rd.x + v_ * rd.y;

    return Ray(origin_ + offset,
               lower_left_corner_ + s * horizontal_ + t * vertical_ - origin_ - offset);
}

void Camera::update(const Point3& lookfrom, const Point3& lookat, const Vec3& vup) {
    origin_ = lookfrom;
    lookat_ = lookat;
    vup_ = vup;
    rebuild();
}

void Camera::translate(const Vec3& delta) {
    origin_ += delta;
    lookat_ += delta;
    rebuild();
}

void Camera::rotate(float yaw, float pitch) {
    // Clamp pitch to prevent gimbal lock
    pitch = glm::clamp(pitch, -glm::pi<float>()/2 + 0.1f, glm::pi<float>()/2 - 0.1f);
    
    // Get current direction vector
    Vec3 direction = glm::normalize(lookat_ - origin_);
    
    // Apply yaw rotation (around up vector)
    Vec3 right = glm::normalize(glm::cross(direction, vup_));
    Vec3 up = glm::normalize(glm::cross(right, direction));
    
    // Simple rotation using cross products
    // Yaw rotation around up vector
    direction = direction * cos(yaw) + glm::cross(up, direction) * sin(yaw);
    
    // Pitch rotation around right vector
    right = glm::normalize(glm::cross(direction, vup_));
    direction = direction * cos(pitch) + glm::cross(right, direction) * sin(pitch);
    
    // Update look-at target
    lookat_ = origin_ + direction;
    rebuild();
}

void Camera::set_fov(float vfov) {
    vfov_ = vfov;
    rebuild();
}

void Camera::rebuild() {
    auto theta = glm::radians(vfov_);
    auto h = tan(theta / 2);
    auto viewport_height = 2.0f * h;
    auto viewport_width = aspect_ratio_ * viewport_height;

    w_ = glm::normalize(origin_ - lookat_);
    u_ = glm::normalize(glm::cross(vup_, w_));
    v_ = glm::cross(w_, u_);

    horizontal_ = focus_dist_ * viewport_width * u_;
    vertical_ = focus_dist_ * viewport_height * v_;
    lower_left_corner_ = origin_ - horizontal_ / 2.0f - vertical_ / 2.0f - focus_dist_ * w_;

    lens_radius_ = aperture_ / 2.0f;
}

} // namespace core
} // namespace raytracer
