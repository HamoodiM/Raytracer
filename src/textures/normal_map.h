/**
 * @file normal_map.h
 * @brief Normal map texture implementation for surface detail
 * 
 * This class handles normal maps that perturb surface normals to add
 * detail without additional geometry. Normal maps are encoded as RGB
 * where (0.5, 0.5, 1.0) represents an unperturbed normal.
 */

#pragma once

#include "texture.h"
#include "../geometry/primitive.h"
#include <memory>

namespace raytracer {
namespace textures {

/**
 * @brief Normal map texture for surface normal perturbation
 * 
 * Samples normal maps and transforms them from tangent space to world space
 * using the TBN (Tangent-Bitangent-Normal) matrix.
 */
class NormalMap : public Texture {
public:
    /**
     * @brief Constructs a normal map from an image texture
     * 
     * @param normal_texture Image texture containing the normal map
     * @param strength Strength of normal perturbation (0.0 = no effect, 1.0 = full effect)
     */
    NormalMap(std::shared_ptr<Texture> normal_texture, float strength = 1.0f);
    
    /**
     * @brief Samples the normal map and returns the perturbed normal in world space
     * 
     * @param rec Hit record containing UV coordinates and tangent space vectors
     * @return Perturbed normal vector in world space
     */
    Vec3 sample_normal(const geometry::HitRecord& rec) const;
    
    /**
     * @brief Samples a color value (for compatibility with Texture interface)
     * 
     * @param u Horizontal texture coordinate in range [0, 1]
     * @param v Vertical texture coordinate in range [0, 1]
     * @return RGB color value (same as the underlying normal map)
     */
    Color sample(float u, float v) const override;

private:
    std::shared_ptr<Texture> normal_texture_;
    float strength_;
    
    /**
     * @brief Transforms a normal from tangent space to world space
     * 
     * @param tangent_space_normal Normal in tangent space
     * @param rec Hit record containing TBN matrix
     * @return Normal in world space
     */
    Vec3 transform_to_world_space(const Vec3& tangent_space_normal, 
                                 const geometry::HitRecord& rec) const;
};

} // namespace textures
} // namespace raytracer
