#include "renderer.h"
#include "../common.h"
#include "../core/utils.h"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <atomic>

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

    const int width = framebuffer_.width();
    const int height = framebuffer_.height();
    std::atomic<int> rows_done(0);
    const int report_every = std::max(1, height / 10);

    // Rows are independent: each pixel only writes its own framebuffer
    // entry, and the RNG is thread_local and reseeded per pixel, so the
    // image does not depend on how rows are spread over threads.
    // schedule(dynamic): rows through glass/metal cost more than sky rows.
#if defined(_OPENMP)
#pragma omp parallel for schedule(dynamic, 1)
#endif
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            core::seed_rng_for_pixel(x, y, 0, seed_);
            Color pixel_color(0, 0, 0);
            
            for (int s = 0; s < samples_per_pixel_; ++s) {
                auto u = (x + core::random_float()) / (width - 1);
                auto v = (y + core::random_float()) / (height - 1);
                
                core::Ray ray = camera.get_ray(u, v);
                pixel_color += integrator_.trace(ray, scene, max_depth_);
            }
            
            pixel_color /= static_cast<float>(samples_per_pixel_);
            framebuffer_.set_pixel(x, y, pixel_color);
        }
        
        // Progress indicator
        int done = ++rows_done;
        if (done % report_every == 0) {
#if defined(_OPENMP)
#pragma omp critical(progress_output)
#endif
            std::cout << "Progress: " << (done * 100 / height) << "%" << std::endl;
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
    std::cout << "Rendering completed in " << duration.count() << " seconds" << std::endl;
}

} // namespace rendering
} // namespace raytracer
