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

private:
    Color background(const core::Ray& ray) const;
    Color direct_lighting(const geometry::HitRecord& rec, const Color& albedo,
                          const Vec3& shading_normal, const core::Scene& scene) const;
};

} // namespace rendering
} // namespace raytracer
