#include "framebuffer.h"
#include "../common.h"
#include <fstream>
#include <iostream>
#include <algorithm>

namespace raytracer {
namespace rendering {

Framebuffer::Framebuffer(int width, int height) 
    : width_(width), height_(height), pixels_(width * height) {
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

void Framebuffer::clear(const Color& color) {
    std::fill(pixels_.begin(), pixels_.end(), color);
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
            float r = sqrt(pixel.r);
            float g = sqrt(pixel.g);
            float b = sqrt(pixel.b);
            
            int ir = static_cast<int>(256 * std::clamp(r, 0.0f, 0.999f));
            int ig = static_cast<int>(256 * std::clamp(g, 0.0f, 0.999f));
            int ib = static_cast<int>(256 * std::clamp(b, 0.0f, 0.999f));
            
            file << ir << " " << ig << " " << ib << "\n";
        }
    }
    
    file.close();
    std::cout << "Image saved to " << filename << std::endl;
}

} // namespace rendering
} // namespace raytracer
