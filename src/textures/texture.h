/**
 * @file texture.h
 * @brief Abstract base class for texture sampling in the raytracer
 * 
 * This file defines the Texture interface that all texture types must implement.
 * Textures provide color values at specific UV coordinates for material shading.
 */

#pragma once

#include "../common.h"

namespace raytracer {
namespace textures {

/**
 * @brief Abstract base class for all texture types
 * 
 * Textures provide color sampling at UV coordinates for use in material shading.
 * UV coordinates are expected to be in the range [0, 1] for both u and v.
 */
class Texture {
public:
    virtual ~Texture() = default;
    
    /**
     * @brief Samples a color value from the texture at the given UV coordinates
     * 
     * @param u Horizontal texture coordinate in range [0, 1]
     * @param v Vertical texture coordinate in range [0, 1]
     * @return RGB color value at the specified coordinates
     */
    virtual Color sample(float u, float v) const = 0;
};

} // namespace textures
} // namespace raytracer
