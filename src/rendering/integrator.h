#pragma once

#include "../common.h"
#include "../core/ray.h"
#include "../core/scene.h"
#include <random>

namespace raytracer {
namespace rendering {

class Integrator {
public:
    Integrator() = default;
    
    /**
     * @brief Estimates the radiance arriving along a camera ray
     *
     * Iterative path tracer with next-event estimation on diffuse vertices.
     *
     * @param max_depth Maximum number of surface interactions per path
     */
    Color trace(const core::Ray& ray, const core::Scene& scene, int max_depth) const;

    /**
     * @brief Bounce count after which Russian roulette may end paths
     *
     * @param bounces Number of scattering events before roulette starts;
     *        0 or less disables Russian roulette.
     */
    void set_russian_roulette_start(int bounces) { rr_start_ = bounces; }
    int russian_roulette_start() const { return rr_start_; }

private:
    int rr_start_ = 3;

    Color background(const core::Ray& ray) const;
    Color direct_lighting(const geometry::HitRecord& rec, const Color& albedo,
                          const Vec3& shading_normal, const core::Scene& scene) const;
};

} // namespace rendering
} // namespace raytracer
