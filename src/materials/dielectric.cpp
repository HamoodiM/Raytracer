#include "dielectric.h"
#include "../common.h"
#include "../core/utils.h"
#include "../geometry/primitive.h"
#include <glm/glm.hpp>

namespace raytracer {
namespace materials {

namespace {
    float reflectance(float cosine, float ref_idx) {
        // Schlick's approximation
        auto r0 = (1 - ref_idx) / (1 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
}

bool Dielectric::scatter(const core::Ray& ray_in, const geometry::HitRecord& rec, 
                        Color& attenuation, core::Ray& scattered) const {
    attenuation = Color(1.0, 1.0, 1.0);
    float refraction_ratio = rec.front_face ? (1.0f/ir_) : ir_;
    
    Vec3 unit_direction = glm::normalize(ray_in.direction());
    float cos_theta = fmin(glm::dot(-unit_direction, rec.normal), 1.0f);
    float sin_theta = sqrt(1.0f - cos_theta * cos_theta);
    
    bool cannot_refract = refraction_ratio * sin_theta > 1.0;
    Vec3 direction;
    
    if (cannot_refract || reflectance(cos_theta, refraction_ratio) > core::random_float())
        direction = core::reflect(unit_direction, rec.normal);
    else
        direction = glm::refract(unit_direction, rec.normal, refraction_ratio);
    
    scattered = core::Ray(rec.point, direction);
    return true;
}

} // namespace materials
} // namespace raytracer
