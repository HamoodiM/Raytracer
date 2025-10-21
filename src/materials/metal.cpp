#include "metal.h"

namespace raytracer {
namespace materials {

bool Metal::scatter(const core::Ray& ray_in, const geometry::HitRecord& rec, 
                    Color& attenuation, core::Ray& scattered) const {
    // TODO: Implement metal reflection
    return false;
}

} // namespace materials
} // namespace raytracer
