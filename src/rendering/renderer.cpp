#include "renderer.h"
#include "../common.h"
#include "../core/utils.h"
#include <iostream>
#include <chrono>

namespace raytracer {
namespace rendering {

Renderer::Renderer(int width, int height, int samples_per_pixel, int max_depth)
    : framebuffer_(width, height), samples_per_pixel_(samples_per_pixel), max_depth_(max_depth) {
}

void Renderer::render(const core::Camera& camera, const core::Scene& scene) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::cout << "Rendering " << framebuffer_.width() << "x" << framebuffer_.height() 
              << " with " << samples_per_pixel_ << " samples per pixel..." << std::endl;
    
    framebuffer_.clear();
    
    for (int y = 0; y < framebuffer_.height(); ++y) {
        for (int x = 0; x < framebuffer_.width(); ++x) {
            Color pixel_color(0, 0, 0);
            
            for (int s = 0; s < samples_per_pixel_; ++s) {
                auto u = (x + core::random_float()) / (framebuffer_.width() - 1);
                auto v = (y + core::random_float()) / (framebuffer_.height() - 1);
                
                core::Ray ray = camera.get_ray(u, v);
                pixel_color += integrator_.trace(ray, scene, max_depth_);
            }
            
            pixel_color /= static_cast<float>(samples_per_pixel_);
            framebuffer_.set_pixel(x, y, pixel_color);
        }
        
        // Progress indicator
        if (y % (framebuffer_.height() / 10) == 0) {
            std::cout << "Progress: " << (y * 100 / framebuffer_.height()) << "%" << std::endl;
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
    std::cout << "Rendering completed in " << duration.count() << " seconds" << std::endl;
}

} // namespace rendering
} // namespace raytracer
