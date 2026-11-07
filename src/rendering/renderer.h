#pragma once

#include "framebuffer.h"
#include "integrator.h"
#include "../core/camera.h"
#include "../core/scene.h"
#include <cstdint>

namespace raytracer {
namespace rendering {

class Renderer {
public:
    Renderer(int width, int height, int samples_per_pixel = 100, int max_depth = 50);
    
    void render(const core::Camera& camera, const core::Scene& scene);
    const Framebuffer& framebuffer() const { return framebuffer_; }
    
    void set_samples_per_pixel(int samples) { samples_per_pixel_ = samples; }
    void set_max_depth(int depth) { max_depth_ = depth; }
    void set_seed(uint64_t seed) { seed_ = seed; }
    Integrator& integrator() { return integrator_; }

private:
    Framebuffer framebuffer_;
    Integrator integrator_;
    int samples_per_pixel_;
    int max_depth_;
    uint64_t seed_ = 0;
};

} // namespace rendering
} // namespace raytracer
