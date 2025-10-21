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
    
    Color trace(const core::Ray& ray, const core::Scene& scene, int depth) const;

private:
    Color ray_color(const core::Ray& ray, const core::Scene& scene, int depth) const;
};

} // namespace rendering
} // namespace raytracer
