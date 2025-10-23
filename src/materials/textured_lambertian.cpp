/**
 * @file textured_lambertian.cpp
 * @brief Implementation of textured Lambertian material with normal mapping
 */

#include "textured_lambertian.h"
#include "../common.h"
#include "../core/utils.h"
#include "../geometry/primitive.h"

namespace raytracer {
namespace materials {

TexturedLambertian::TexturedLambertian(std::shared_ptr<textures::Texture> albedo_texture,
                                      std::shared_ptr<textures::NormalMap> normal_map)
    : albedo_texture_(albedo_texture), normal_map_(normal_map) {
}

bool TexturedLambertian::scatter(const core::Ray& ray_in, const geometry::HitRecord& rec, 
                                Color& attenuation, core::Ray& scattered) const {
    // Get the surface normal (potentially perturbed by normal map)
    Vec3 surface_normal = rec.normal;
    if (normal_map_) {
        surface_normal = normal_map_->sample_normal(rec);
    }
    
    // Generate scatter direction using the (possibly perturbed) normal
    auto scatter_direction = surface_normal + core::random_unit_vector();

    // Catch degenerate scatter direction
    if (glm::length(scatter_direction) < 1e-8f) {
        scatter_direction = surface_normal;
    }

    scattered = core::Ray(rec.point, scatter_direction);
    
    // Sample albedo from texture
    attenuation = albedo_texture_->sample(rec.u, rec.v);
    
    return true;
}

} // namespace materials
} // namespace raytracer
