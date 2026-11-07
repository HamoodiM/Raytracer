#pragma once

#include "../common.h"
#include "../core/ray.h"

namespace raytracer {
namespace geometry {
    struct HitRecord;
}

namespace materials {

class Material {
public:
    virtual ~Material() = default;
    virtual bool scatter(const core::Ray& ray_in, const geometry::HitRecord& rec, 
                        Color& attenuation, core::Ray& scattered) const = 0;
    virtual Color emit() const { return Color(0, 0, 0); }

    /**
     * @brief True if this emitter is also registered as a scene light
     *
     * Such emitters are already counted by next-event estimation at the
     * previous diffuse vertex, so the integrator must not add their emission
     * again when a diffuse bounce ray happens to hit them.
     */
    virtual bool emission_sampled_by_lights() const { return false; }

    /**
     * @brief BRDF data for next-event estimation (direct light sampling)
     *
     * Diffuse materials return true and fill in their albedo and shading
     * normal; the integrator then evaluates f = albedo / pi for each light
     * sample. Mirror/glass (delta) and fuzzy-metal lobes are not diffuse, so
     * they return false and receive light only through their scattered ray.
     */
    virtual bool diffuse_brdf(const geometry::HitRecord& rec, Color& albedo,
                              Vec3& shading_normal) const {
        (void)rec; (void)albedo; (void)shading_normal;
        return false;
    }
};

} // namespace materials
} // namespace raytracer
