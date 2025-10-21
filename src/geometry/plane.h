#pragma once

#include "primitive.h"
#include "../materials/material.h"
#include <memory>

namespace raytracer {
namespace geometry {

class Plane : public Primitive {
public:
    Plane() = default;
    Plane(const Point3& point, const Vec3& normal, std::shared_ptr<materials::Material> material)
        : point_(point), normal_(normal), material_(material) {}

    bool hit(const core::Ray& ray, float t_min, float t_max, HitRecord& rec) const override;

private:
    Point3 point_;
    Vec3 normal_;
    std::shared_ptr<materials::Material> material_;
};

} // namespace geometry
} // namespace raytracer
