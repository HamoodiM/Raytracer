/**
 * @file progressive_renderer.cpp
 * @brief Implementation of progressive rendering system
 */

#include "progressive_renderer.h"
#include "../common.h"
#include "../core/utils.h"
#include <iostream>

namespace raytracer {
namespace rendering {

ProgressiveRenderer::ProgressiveRenderer(int width, int height, int max_depth)
    : framebuffer_(width, height), sample_count_(0), target_samples_(100), max_depth_(max_depth) {
}

bool ProgressiveRenderer::render_sample(const core::Camera& camera, const core::Scene& scene) {
    if (sample_count_ >= target_samples_) {
        return false; // Rendering complete
    }
    
    render_single_sample(camera, scene);
    sample_count_++;
    
    // Update display every 10 samples for real-time preview
    if (sample_count_ % 10 == 0 || sample_count_ == target_samples_) {
        framebuffer_.update_display(sample_count_);
    }
    
    return sample_count_ < target_samples_;
}

void ProgressiveRenderer::reset() {
    sample_count_ = 0;
    framebuffer_.clear();
}

void ProgressiveRenderer::render_single_sample(const core::Camera& camera, const core::Scene& scene) {
    // Render one sample per pixel
    for (int y = 0; y < framebuffer_.height(); ++y) {
        for (int x = 0; x < framebuffer_.width(); ++x) {
            // Generate random UV coordinates for this sample
            auto u = (x + core::random_float()) / (framebuffer_.width() - 1);
            auto v = (y + core::random_float()) / (framebuffer_.height() - 1);
            
            // Cast ray and trace
            core::Ray ray = camera.get_ray(u, v);
            Color color = integrator_.trace(ray, scene, max_depth_);
            
            // Add sample to accumulation buffer
            framebuffer_.add_sample(x, y, color);
        }
    }
}

} // namespace rendering
} // namespace raytracer
