/**
 * @file checker_texture.h
 * @brief Procedural checker pattern texture implementation
 * 
 * This class generates a checkerboard pattern using two colors
 * with configurable frequency and phase offset.
 */

#pragma once

#include "texture.h"

namespace raytracer {
namespace textures {

/**
 * @brief Procedural checkerboard texture
 * 
 * Generates a checkerboard pattern alternating between two colors.
 * The frequency parameter controls how many squares fit in the [0,1] UV range.
 */
class CheckerTexture : public Texture {
public:
    /**
     * @brief Constructs a checker texture with two colors
     * 
     * @param color1 First color of the checker pattern
     * @param color2 Second color of the checker pattern
     * @param frequency Number of checker squares per UV unit (default: 10)
     */
    CheckerTexture(const Color& color1, const Color& color2, float frequency = 10.0f);
    
    /**
     * @brief Samples the checker pattern at UV coordinates
     * 
     * @param u Horizontal texture coordinate in range [0, 1]
     * @param v Vertical texture coordinate in range [0, 1]
     * @return Either color1 or color2 based on checker pattern
     */
    Color sample(float u, float v) const override;

private:
    Color color1_;
    Color color2_;
    float frequency_;
};

} // namespace textures
} // namespace raytracer
