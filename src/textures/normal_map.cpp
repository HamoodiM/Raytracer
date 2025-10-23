/**
 * @file normal_map.cpp
 * @brief Implementation of normal map texture for surface detail
 */

#include "normal_map.h"
#include <glm/glm.hpp>

namespace raytracer {
namespace textures {

NormalMap::NormalMap(std::shared_ptr<Texture> normal_texture, float strength)
    : normal_texture_(normal_texture), strength_(strength) {
}

Vec3 NormalMap::sample_normal(const geometry::HitRecord& rec) const {
    // Sample the normal map in tangent space
    Color normal_color = normal_texture_->sample(rec.u, rec.v);
    
    // Convert from [0, 1] range to [-1, 1] range
    // Normal maps encode (0.5, 0.5, 1.0) as unperturbed normal
    Vec3 tangent_space_normal(
        2.0f * normal_color.r - 1.0f,
        2.0f * normal_color.g - 1.0f,
        2.0f * normal_color.b - 1.0f
    );
    
    // Apply strength factor
    tangent_space_normal = glm::normalize(
        Vec3(0, 0, 1) + strength_ * (tangent_space_normal - Vec3(0, 0, 1))
    );
    
    // Transform from tangent space to world space
    return transform_to_world_space(tangent_space_normal, rec);
}

Color NormalMap::sample(float u, float v) const {
    // For compatibility with Texture interface, just return the underlying texture
    return normal_texture_->sample(u, v);
}

Vec3 NormalMap::transform_to_world_space(const Vec3& tangent_space_normal, 
                                        const geometry::HitRecord& rec) const {
    // Construct TBN matrix: [T, B, N] where T=Tangent, B=Bitangent, N=Normal
    // The transformation is: world_normal = TBN * tangent_normal
    
    // Create TBN matrix (transpose for proper transformation)
    glm::mat3 tbn_matrix(
        rec.tangent.x, rec.bitangent.x, rec.normal.x,
        rec.tangent.y, rec.bitangent.y, rec.normal.y,
        rec.tangent.z, rec.bitangent.z, rec.normal.z
    );
    
    // Transform the normal from tangent space to world space
    Vec3 world_normal = tbn_matrix * tangent_space_normal;
    
    return glm::normalize(world_normal);
}

} // namespace textures
} // namespace raytracer
