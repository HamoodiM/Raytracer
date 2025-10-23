/**
 * @file checker_texture.cpp
 * @brief Implementation of procedural checker pattern texture
 */

#include "checker_texture.h"
#include <cmath>

namespace raytracer {
namespace textures {

CheckerTexture::CheckerTexture(const Color& color1, const Color& color2, float frequency)
    : color1_(color1), color2_(color2), frequency_(frequency) {
}

Color CheckerTexture::sample(float u, float v) const {
    // Scale UV coordinates by frequency
    float scaled_u = u * frequency_;
    float scaled_v = v * frequency_;
    
    // Use floor to determine which checker square we're in
    int u_check = static_cast<int>(std::floor(scaled_u));
    int v_check = static_cast<int>(std::floor(scaled_v));
    
    // XOR the checker indices to create the pattern
    // If both are even or both are odd, use color1, otherwise color2
    bool use_color1 = ((u_check + v_check) % 2) == 0;
    
    return use_color1 ? color1_ : color2_;
}

} // namespace textures
} // namespace raytracer
