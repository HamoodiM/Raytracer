#pragma once

#include "material.h"

namespace raytracer {
namespace materials {

class Emissive : public Material {
public:
    /**
     * @param emit Emitted radiance
     * @param sampled_by_lights Set when this surface is the visible geometry
     *        of a Light in the scene (e.g. an area light), so the integrator
     *        knows next-event estimation already accounts for it.
     */
    explicit Emissive(const Color& emit, bool sampled_by_lights = false)
        : emit_(emit), sampled_by_lights_(sampled_by_lights) {}

    bool scatter(const core::Ray& ray_in, const geometry::HitRecord& rec, 
                Color& attenuation, core::Ray& scattered) const override;
    Color emit() const override { return emit_; }
    bool emission_sampled_by_lights() const override { return sampled_by_lights_; }

private:
    Color emit_;
    bool sampled_by_lights_;
};

} // namespace materials
} // namespace raytracer
