#include "integrator.h"
#include "../common.h"
#include "../core/utils.h"
#include <glm/glm.hpp>

namespace raytracer {
namespace rendering {

Color Integrator::trace(const core::Ray& ray, const core::Scene& scene, int depth) const {
    return ray_color(ray, scene, depth);
}

Color Integrator::ray_color(const core::Ray& ray, const core::Scene& scene, int depth) const {
    geometry::HitRecord rec;

    // If we've exceeded the ray bounce limit, no more light is gathered
    if (depth <= 0) {
        return Color(0, 0, 0);
    }

    if (scene.hit(ray, 0.001f, std::numeric_limits<float>::infinity(), rec)) {
        core::Ray scattered;
        Color attenuation;
        if (rec.material->scatter(ray, rec, attenuation, scattered)) {
            return attenuation * ray_color(scattered, scene, depth - 1);
        }
        return Color(0, 0, 0);
    }

    // Background gradient
    Vec3 unit_direction = glm::normalize(ray.direction());
    auto t = 0.5f * (unit_direction.y + 1.0f);
    return (1.0f - t) * Color(1.0f, 1.0f, 1.0f) + t * Color(0.5f, 0.7f, 1.0f);
}

} // namespace rendering
} // namespace raytracer
