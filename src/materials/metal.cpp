#include "metal.h"
#include "../common.h"
#include "../core/utils.h"
#include "../geometry/primitive.h"
#include <glm/glm.hpp>

namespace raytracer {
namespace materials {

bool Metal::scatter(const core::Ray& ray_in, const geometry::HitRecord& rec, 
                    Color& attenuation, core::Ray& scattered) const {
    Vec3 reflected = core::reflect(glm::normalize(ray_in.direction()), rec.normal);
    scattered = core::Ray(rec.point, reflected + fuzz_ * core::random_in_unit_sphere());
    attenuation = albedo_;
    return (glm::dot(scattered.direction(), rec.normal) > 0);
}

} // namespace materials
} // namespace raytracer
