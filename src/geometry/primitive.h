#pragma once

#include "../common.h"
#include "../core/ray.h"
#include "../materials/material.h"
#include <memory>

namespace raytracer {
namespace geometry {

struct HitRecord {
    Point3 point;
    Vec3 normal;
    std::shared_ptr<materials::Material> material;
    float t;
    bool front_face;

    inline void set_face_normal(const core::Ray& ray, const Vec3& outward_normal) {
        front_face = glm::dot(ray.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class Primitive {
public:
    virtual ~Primitive() = default;
    virtual bool hit(const core::Ray& ray, float t_min, float t_max, HitRecord& rec) const = 0;
};

} // namespace geometry
} // namespace raytracer
