/**
 * @file image_texture.cpp
 * @brief Implementation of image-based texture loading and sampling
 */

#include "image_texture.h"
#include <iostream>
#include <stdexcept>
#include <glm/glm.hpp>

// stb_image implementation
#define STB_IMAGE_IMPLEMENTATION
#include "../../external/stb_image.h"

namespace raytracer {
namespace textures {

ImageTexture::ImageTexture(const std::string& filename) 
    : data_(nullptr), width_(0), height_(0), channels_(0) {
    
    // Load image with stb_image
    data_ = stbi_load(filename.c_str(), &width_, &height_, &channels_, 3);
    
    if (!data_) {
        throw std::runtime_error("Failed to load image: " + filename);
    }
    
    std::cout << "Loaded texture: " << filename 
              << " (" << width_ << "x" << height_ << ", " << channels_ << " channels)" << std::endl;
}

ImageTexture::ImageTexture(const unsigned char* data, int width, int height)
    : data_(nullptr), width_(width), height_(height), channels_(3) {
    
    // Copy the provided data
    int size = width * height * channels_;
    data_ = new unsigned char[size];
    std::memcpy(data_, data, size);
}

ImageTexture::~ImageTexture() {
    if (data_) {
        stbi_image_free(data_);
    }
}

Color ImageTexture::sample(float u, float v) const {
    if (!data_) {
        return Color(1.0f, 0.0f, 1.0f); // Magenta for missing texture
    }
    
    // Clamp UV coordinates to [0, 1] range
    u = clamp_coordinate(u);
    v = clamp_coordinate(v);
    
    // Convert UV to pixel coordinates
    float x = u * (width_ - 1);
    float y = v * (height_ - 1);
    
    // Get integer pixel coordinates for bilinear filtering
    int x0 = static_cast<int>(x);
    int y0 = static_cast<int>(y);
    int x1 = std::min(x0 + 1, width_ - 1);
    int y1 = std::min(y0 + 1, height_ - 1);
    
    // Get fractional parts for interpolation
    float fx = x - x0;
    float fy = y - y0;
    
    // Sample four neighboring pixels
    auto get_pixel = [this](int x, int y) -> Color {
        int index = (y * width_ + x) * channels_;
        return Color(
            data_[index] / 255.0f,
            data_[index + 1] / 255.0f,
            data_[index + 2] / 255.0f
        );
    };
    
    Color c00 = get_pixel(x0, y0);
    Color c10 = get_pixel(x1, y0);
    Color c01 = get_pixel(x0, y1);
    Color c11 = get_pixel(x1, y1);
    
    // Bilinear interpolation
    Color c0 = c00 * (1.0f - fx) + c10 * fx;
    Color c1 = c01 * (1.0f - fx) + c11 * fx;
    
    return c0 * (1.0f - fy) + c1 * fy;
}

float ImageTexture::clamp_coordinate(float coord) const {
    // Simple clamp to [0, 1] range
    return glm::clamp(coord, 0.0f, 1.0f);
}

} // namespace textures
} // namespace raytracer
