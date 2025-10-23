#include "lambertian.h"
#include "../common.h"
#include "../core/utils.h"
#include "../geometry/primitive.h"

namespace raytracer {
namespace materials {

Lambertian::Lambertian(const Color& albedo) 
    : solid_albedo_(albedo), use_texture_(false) {
}

Lambertian::Lambertian(std::shared_ptr<textures::Texture> texture)
    : texture_(texture), use_texture_(true) {
}

bool Lambertian::scatter(const core::Ray& ray_in, const geometry::HitRecord& rec, 
                        Color& attenuation, core::Ray& scattered) const {
    auto scatter_direction = rec.normal + core::random_unit_vector();

    // Catch degenerate scatter direction
    if (glm::length(scatter_direction) < 1e-8f) {
        scatter_direction = rec.normal;
    }

    scattered = core::Ray(rec.point, scatter_direction);
    
    // Sample albedo from texture or use solid color
    if (use_texture_) {
        attenuation = texture_->sample(rec.u, rec.v);
    } else {
        attenuation = solid_albedo_;
    }
    
    return true;
}

} // namespace materials
} // namespace raytracer
