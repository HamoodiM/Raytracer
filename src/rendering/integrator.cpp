#include "integrator.h"
#include "../common.h"
#include "../core/utils.h"
#include "../core/light.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <limits>

namespace raytracer {
namespace rendering {

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
        if (cos_theta <= 0.0f || glm::dot(rec.geometric_normal, sample.direction) <= 0.0f) continue;

        // Shadow ray
        core::Ray shadow_ray(rec.point, sample.direction);
        geometry::HitRecord shadow_rec;
        if (scene.hit(shadow_ray, 0.001f, sample.distance - 0.001f, shadow_rec)) continue;

        direct += brdf * sample.radiance * cos_theta / sample.pdf;
    }

    return direct;
}

Color Integrator::trace(const core::Ray& camera_ray, const core::Scene& scene, int max_depth) const {
    Color radiance(0.0f);
    Color throughput(1.0f);    // product of BRDF * cos / pdf along the path
    core::Ray ray = camera_ray;

    // Emission found by a bounce ray is only added when the previous vertex
    // did NOT already sample the lights (camera ray, mirror, glass, fuzzy
    // metal). After a diffuse vertex, light-sampled emitters were counted by
    // next-event estimation, and adding them again would count them twice.
    bool count_light_emission = true;

    for (int depth = 0; depth < max_depth; ++depth) {
        geometry::HitRecord rec;
        if (!scene.hit(ray, 0.001f, std::numeric_limits<float>::infinity(), rec)) {
            // The sky is never sampled by NEE, so it is always counted.
            radiance += throughput * scene.background(ray.direction());
            break;
        }

        const materials::Material& material = *rec.material;
        if (count_light_emission || !material.emission_sampled_by_lights())
            radiance += throughput * material.emit();

        core::Ray scattered;
        Color attenuation;
        if (!material.scatter(ray, rec, attenuation, scattered))
            break;  // pure emitter, or absorbed (e.g. metal ray below the surface)

        Color albedo;
        Vec3 shading_normal;
        if (material.diffuse_brdf(rec, albedo, shading_normal)) {
            radiance += throughput * direct_lighting(rec, albedo, shading_normal, scene);
            count_light_emission = false;
        } else {
            count_light_emission = true;
        }

        // Lambertian: cosine-weighted sampling makes BRDF*cos/pdf = albedo.
        // Metal/glass: attenuation is the specular weight.
        throughput *= attenuation;
        ray = scattered;

        // Russian roulette: after a few bounces, continue the path with
        // probability p and divide the throughput by p. The expected value is
        // unchanged (p * X/p + (1 - p) * 0 = X), so the image stays unbiased,
        // but dim paths that contribute little are ended early.
        // p follows the throughput (capped at 0.95 so no path is immortal).
        if (rr_start_ > 0 && depth + 1 >= rr_start_) {
            float p = glm::min(glm::max(throughput.r, glm::max(throughput.g, throughput.b)), 0.95f);
            if (core::random_float() >= p)
                break;
            throughput /= p;
        }
    }

    return radiance;
}

} // namespace rendering
} // namespace raytracer
