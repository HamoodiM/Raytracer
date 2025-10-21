#include "dielectric.h"

namespace raytracer {
namespace materials {

bool Dielectric::scatter(const core::Ray& ray_in, const geometry::HitRecord& rec, 
                        Color& attenuation, core::Ray& scattered) const {
    // TODO: Implement dielectric refraction
    return false;
}

} // namespace materials
} // namespace raytracer
