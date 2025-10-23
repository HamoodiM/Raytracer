/**
 * @file image_texture.h
 * @brief Image-based texture implementation using stb_image
 * 
 * This class loads image files (PNG, JPG, etc.) and provides color sampling
 * at UV coordinates with proper filtering and wrapping modes.
 */

#pragma once

#include "texture.h"
#include <string>
#include <vector>

namespace raytracer {
namespace textures {

/**
 * @brief Texture implementation that loads and samples from image files
 * 
 * Supports common image formats (PNG, JPG, BMP, TGA) through stb_image.
 * Provides bilinear filtering and UV coordinate wrapping.
 */
class ImageTexture : public Texture {
public:
    /**
     * @brief Constructs an image texture from a file path
     * 
     * @param filename Path to the image file to load
     * @throws std::runtime_error if the image cannot be loaded
     */
    explicit ImageTexture(const std::string& filename);
    
    /**
     * @brief Constructs an image texture from raw pixel data
     * 
     * @param data Raw RGB pixel data
     * @param width Image width in pixels
     * @param height Image height in pixels
     */
    ImageTexture(const unsigned char* data, int width, int height);
    
    ~ImageTexture() override;
    
    /**
     * @brief Samples the image texture with bilinear filtering
     * 
     * @param u Horizontal texture coordinate in range [0, 1]
     * @param v Vertical texture coordinate in range [0, 1]
     * @return RGB color value with bilinear interpolation
     */
    Color sample(float u, float v) const override;

private:
    unsigned char* data_;
    int width_;
    int height_;
    int channels_;
    
    /**
     * @brief Clamps UV coordinates to valid range and applies wrapping
     * 
     * @param coord UV coordinate to clamp
     * @return Clamped coordinate in range [0, 1]
     */
    float clamp_coordinate(float coord) const;
};

} // namespace textures
} // namespace raytracer
