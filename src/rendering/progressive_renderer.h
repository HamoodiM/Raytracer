/**
 * @file progressive_renderer.h
 * @brief Progressive rendering system for real-time preview
 * 
 * This class implements progressive rendering that accumulates samples
 * over time and provides real-time updates to the display.
 */

#pragma once

#include "framebuffer.h"
#include "integrator.h"
#include "../core/camera.h"
#include "../core/scene.h"
#include <memory>

namespace raytracer {
namespace rendering {

class ProgressiveRenderer {
public:
    /**
     * @brief Constructs a progressive renderer
     * 
     * @param width Image width in pixels
     * @param height Image height in pixels
     * @param max_depth Maximum ray bounce depth
     */
    ProgressiveRenderer(int width, int height, int max_depth);
    
    /**
     * @brief Renders one sample per pixel and updates the display
     * 
     * @param camera Camera to render from
     * @param scene Scene to render
     * @return True if rendering should continue, false if complete
     */
    bool render_sample(const core::Camera& camera, const core::Scene& scene);
    
    /**
     * @brief Gets the current framebuffer for display
     * 
     * @return Reference to the framebuffer
     */
    const Framebuffer& framebuffer() const { return framebuffer_; }
    
    /**
     * @brief Gets the current sample count
     * 
     * @return Number of samples rendered per pixel
     */
    int sample_count() const { return sample_count_; }
    
    /**
     * @brief Resets the renderer to start over
     */
    void reset();
    
    /**
     * @brief Sets the target number of samples per pixel
     * 
     * @param samples Target samples per pixel
     */
    void set_target_samples(int samples) { target_samples_ = samples; }

private:
    Framebuffer framebuffer_;
    Integrator integrator_;
    int sample_count_;
    int target_samples_;
    int max_depth_;
    
    /**
     * @brief Renders a single sample for all pixels
     * 
     * @param camera Camera to render from
     * @param scene Scene to render
     */
    void render_single_sample(const core::Camera& camera, const core::Scene& scene);
};

} // namespace rendering
} // namespace raytracer
