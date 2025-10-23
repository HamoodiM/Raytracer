#include "integrator.h"
#include "../common.h"
#include "../core/utils.h"
#include "../core/light.h"
#include <glm/glm.hpp>

namespace raytracer {
namespace rendering {

Color Integrator::trace(const core::Ray& ray, const core::Scene& scene, int depth) const {
    return ray_color(ray, scene, depth);
}

Color Integrator::direct_lighting(const geometry::HitRecord& rec, const core::Scene& scene) const {
    Color direct = Color(0, 0, 0);
    
    // Sample all lights
    for (const auto& light : scene.lights()) {
        auto sample = light->sample(rec.point);
        
        // Check if light is visible (shadow ray)
        core::Ray shadow_ray(rec.point, sample.direction);
        geometry::HitRecord shadow_rec;
        bool in_shadow = scene.hit(shadow_ray, 0.001f, sample.distance - 0.001f, shadow_rec);
        
        if (!in_shadow) {
            // Lambertian BRDF: albedo / pi * cos(theta)
            float cos_theta = glm::max(glm::dot(rec.normal, sample.direction), 0.0f);
            direct += sample.radiance * cos_theta / sample.pdf;
        }
    }
    
    return direct;
}

Color Integrator::ray_color(const core::Ray& ray, const core::Scene& scene, int depth) const {
    if (depth <= 0)
        return Color(0, 0, 0);
    
    geometry::HitRecord rec;
    if (scene.hit(ray, 0.001f, std::numeric_limits<float>::infinity(), rec)) {
        core::Ray scattered;
        Color attenuation;
        Color emitted = rec.material->emit();
        
        // If material doesn't scatter (emissive), return emission only
        if (!rec.material->scatter(ray, rec, attenuation, scattered))
            return emitted;
        
        // Direct lighting (Next Event Estimation)
        Color direct = direct_lighting(rec, scene);
        
        // Indirect lighting (recursive path tracing)
        Color indirect = attenuation * ray_color(scattered, scene, depth - 1);
        
        return emitted + attenuation * direct + indirect;
    }
    
    // Background (sky) for debugging
    Vec3 unit_direction = glm::normalize(ray.direction());
    auto t = 0.5f * (unit_direction.y + 1.0f);
    return (1.0f - t) * Color(1.0, 1.0, 1.0) + t * Color(0.5, 0.7, 1.0);
}

} // namespace rendering
} // namespace raytracer
