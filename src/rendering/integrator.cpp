#include "integrator.h"
#include "../common.h"
#include "../core/utils.h"
#include "../core/light.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <limits>

namespace raytracer {
namespace rendering {

Color Integrator::trace(const core::Ray& ray, const core::Scene& scene, int depth) const {
    return ray_color(ray, scene, depth);
}

Color Integrator::direct_lighting(const geometry::HitRecord& rec, const Color& albedo,
                                  const Vec3& shading_normal, const core::Scene& scene) const {
    // Next-event estimation for a Lambertian surface. For each light, one
    // sample gives the estimator
    //     f * L * cos(theta) / pdf,   with f = albedo / pi
    // where pdf is the light sample's density in solid angle (1 for a point
    // light, whose "radiance" is already intensity / d^2).
    const float inv_pi = 1.0f / glm::pi<float>();
    const Color brdf = albedo * inv_pi;
    Color direct(0.0f);

    for (const auto& light : scene.lights()) {
        auto sample = light->sample(rec.point);
        if (!(sample.pdf > 0.0f)) continue;

        float cos_theta = glm::dot(shading_normal, sample.direction);
        // Skip lights behind the shading normal or behind the actual surface
        if (cos_theta <= 0.0f || glm::dot(rec.normal, sample.direction) <= 0.0f) continue;

        // Shadow ray
        core::Ray shadow_ray(rec.point, sample.direction);
        geometry::HitRecord shadow_rec;
        if (scene.hit(shadow_ray, 0.001f, sample.distance - 0.001f, shadow_rec)) continue;

        direct += brdf * sample.radiance * cos_theta / sample.pdf;
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
        
        // Direct lighting (next-event estimation), only for diffuse surfaces.
        // The BRDF (albedo / pi) is applied inside direct_lighting.
        Color direct(0.0f);
        Color albedo;
        Vec3 shading_normal;
        if (rec.material->diffuse_brdf(rec, albedo, shading_normal))
            direct = direct_lighting(rec, albedo, shading_normal, scene);
        
        // Indirect lighting (recursive path tracing)
        Color indirect = attenuation * ray_color(scattered, scene, depth - 1);
        
        return emitted + direct + indirect;
    }
    
    // Background (sky) for debugging
    Vec3 unit_direction = glm::normalize(ray.direction());
    auto t = 0.5f * (unit_direction.y + 1.0f);
    return (1.0f - t) * Color(1.0, 1.0, 1.0) + t * Color(0.5, 0.7, 1.0);
}

} // namespace rendering
} // namespace raytracer
