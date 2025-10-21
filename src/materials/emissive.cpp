#include "emissive.h"

namespace raytracer {
namespace materials {

bool Emissive::scatter(const core::Ray& ray_in, const geometry::HitRecord& rec, 
                      Color& attenuation, core::Ray& scattered) const {
    // TODO: Implement emissive material
    return false;
}

} // namespace materials
} // namespace raytracer
