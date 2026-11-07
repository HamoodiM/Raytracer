#include "light.h"
#include "utils.h"
#include <glm/glm.hpp>

namespace raytracer {
namespace core {

LightSample PointLight::sample(const Point3& hit_point) const {
    LightSample sample;
    sample.position = position_;
    Vec3 to_light = position_ - hit_point;
    sample.distance = glm::length(to_light);
    sample.direction = glm::normalize(to_light);
    sample.radiance = intensity_ / (sample.distance * sample.distance); // Inverse square falloff
    sample.pdf = 1.0f; // Delta distribution for point light
    return sample;
}

LightSample AreaLight::sample(const Point3& hit_point) const {
    // Uniform random point on the rectangle centred on center_ with edge
    // vectors u_ and v_: center + s*u + t*v with s, t in [-0.5, 0.5).
    float s = random_float(-0.5f, 0.5f);
    float t = random_float(-0.5f, 0.5f);
    Point3 light_point = center_ + s * u_ + t * v_;
    
    LightSample sample;
    sample.position = light_point;
    Vec3 to_light = light_point - hit_point;
    sample.distance = glm::length(to_light);
    sample.direction = glm::normalize(to_light);
    
    // Calculate PDF (1 / area of rectangle)
    float area = glm::length(glm::cross(u_, v_));
    Vec3 light_normal = glm::normalize(glm::cross(u_, v_));
    float cos_theta = glm::abs(glm::dot(light_normal, -sample.direction));
    
    sample.radiance = emission_;
    sample.pdf = (sample.distance * sample.distance) / (area * cos_theta);
    
    return sample;
}

} // namespace core
} // namespace raytracer
