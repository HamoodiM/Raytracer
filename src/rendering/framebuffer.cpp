#include "framebuffer.h"
#include "../common.h"
#include "../core/utils.h"
#include <fstream>
#include <iostream>
#include <algorithm>

namespace raytracer {
namespace rendering {

Framebuffer::Framebuffer(int width, int height) 
    : width_(width), height_(height), pixels_(width * height), accumulation_(width * height) {
}

void Framebuffer::set_pixel(int x, int y, const Color& color) {
    if (x >= 0 && x < width_ && y >= 0 && y < height_) {
        pixels_[y * width_ + x] = color;
    }
}

Color Framebuffer::get_pixel(int x, int y) const {
    if (x >= 0 && x < width_ && y >= 0 && y < height_) {
        return pixels_[y * width_ + x];
    }
    return Color(0, 0, 0);
}

void Framebuffer::add_sample(int x, int y, const Color& color) {
    if (x >= 0 && x < width_ && y >= 0 && y < height_) {
        int index = y * width_ + x;
        accumulation_[index] += color;
    }
}

void Framebuffer::update_display(int sample_count) {
    if (sample_count <= 0) return;
    
    float inv_samples = 1.0f / static_cast<float>(sample_count);
    for (int i = 0; i < width_ * height_; ++i) {
        pixels_[i] = accumulation_[i] * inv_samples;
    }
}

void Framebuffer::clear(const Color& color) {
    std::fill(pixels_.begin(), pixels_.end(), color);
    std::fill(accumulation_.begin(), accumulation_.end(), Color(0, 0, 0));
}

void Framebuffer::save_ppm(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

    file << "P3\n" << width_ << " " << height_ << "\n255\n";
    
    for (int y = height_ - 1; y >= 0; --y) {
        for (int x = 0; x < width_; ++x) {
            const Color& pixel = pixels_[y * width_ + x];
            
            // Apply gamma correction
            Color corrected = core::gamma_correct(pixel, 2.2f);
            
            int ir = static_cast<int>(255.999 * core::clamp(corrected.r, 0.0f, 1.0f));
            int ig = static_cast<int>(255.999 * core::clamp(corrected.g, 0.0f, 1.0f));
            int ib = static_cast<int>(255.999 * core::clamp(corrected.b, 0.0f, 1.0f));
            
            file << ir << " " << ig << " " << ib << "\n";
        }
    }
    
    file.close();
    std::cout << "Image saved to " << filename << std::endl;
}

} // namespace rendering
} // namespace raytracer
