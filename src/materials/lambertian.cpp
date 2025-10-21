#include "lambertian.h"
#include "../common.h"
#include "../core/utils.h"
#include "../geometry/primitive.h"

namespace raytracer {
namespace materials {

bool Lambertian::scatter(const core::Ray& ray_in, const geometry::HitRecord& rec, 
                        Color& attenuation, core::Ray& scattered) const {
    auto scatter_direction = rec.normal + core::random_unit_vector();

    // Catch degenerate scatter direction
    if (glm::length(scatter_direction) < 1e-8f) {
        scatter_direction = rec.normal;
    }

    scattered = core::Ray(rec.point, scatter_direction);
    attenuation = albedo_;
    return true;
}

} // namespace materials
} // namespace raytracer
