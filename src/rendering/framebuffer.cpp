#include "framebuffer.h"
#include "../common.h"
#include "../core/utils.h"
#include "image_io.h"
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

std::vector<uint8_t> Framebuffer::to_rgb8() const {
    // Gamma 2.2 and clamp to [0, 1]; rows are flipped so the output is top-down
    // (row 0 of the framebuffer is the bottom of the image).
    std::vector<uint8_t> rgb(static_cast<size_t>(width_) * height_ * 3);
    size_t i = 0;
    for (int y = height_ - 1; y >= 0; --y) {
        for (int x = 0; x < width_; ++x) {
            Color pixel = pixels_[y * width_ + x];
            for (int c = 0; c < 3; ++c) {
                if (!(pixel[c] >= 0.0f)) pixel[c] = 0.0f;  // also catches NaN
            }
            Color corrected = core::gamma_correct(pixel, 2.2f);
            rgb[i++] = static_cast<uint8_t>(255.999f * core::clamp(corrected.r, 0.0f, 1.0f));
            rgb[i++] = static_cast<uint8_t>(255.999f * core::clamp(corrected.g, 0.0f, 1.0f));
            rgb[i++] = static_cast<uint8_t>(255.999f * core::clamp(corrected.b, 0.0f, 1.0f));
        }
    }
    return rgb;
}

bool Framebuffer::save(const std::string& filename) const {
    if (!write_image(filename, width_, height_, to_rgb8())) {
        std::cerr << "Error: Could not write " << filename << std::endl;
        return false;
    }
    return true;
}

void Framebuffer::save_ppm(const std::string& filename) const {
    if (write_ppm(filename, width_, height_, to_rgb8()))
        std::cout << "Image saved to " << filename << std::endl;
    else
        std::cerr << "Error: Could not open file " << filename << std::endl;
}

} // namespace rendering
} // namespace raytracer
