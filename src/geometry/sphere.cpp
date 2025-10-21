#include "sphere.h"
#include "../common.h"
#include <glm/glm.hpp>

namespace raytracer {
namespace geometry {

bool Sphere::hit(const core::Ray& ray, float t_min, float t_max, HitRecord& rec) const {
    Vec3 oc = ray.origin() - center_;
    auto a = glm::dot(ray.direction(), ray.direction());
    auto half_b = glm::dot(oc, ray.direction());
    auto c = glm::dot(oc, oc) - radius_ * radius_;

    auto discriminant = half_b * half_b - a * c;
    if (discriminant < 0) return false;

    // Find the nearest root that lies in the acceptable range
    auto sqrtd = sqrt(discriminant);
    auto root = (-half_b - sqrtd) / a;
    if (root < t_min || t_max < root) {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || t_max < root)
            return false;
    }

    rec.t = root;
    rec.point = ray.at(rec.t);
    Vec3 outward_normal = (rec.point - center_) / radius_;
    rec.set_face_normal(ray, outward_normal);
    rec.material = material_;

    return true;
}

} // namespace geometry
} // namespace raytracer
