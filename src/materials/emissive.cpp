#include "emissive.h"
#include "../geometry/primitive.h"

namespace raytracer {
namespace materials {

bool Emissive::scatter(const core::Ray& ray_in, const geometry::HitRecord& rec, 
                      Color& attenuation, core::Ray& scattered) const {
    return false; // Emissive materials don't scatter
}

} // namespace materials
} // namespace raytracer
