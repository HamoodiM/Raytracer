#pragma once

#include "primitive.h"
#include "../materials/material.h"
#include <memory>

namespace raytracer {
namespace geometry {

class Sphere : public Primitive {
public:
    Sphere() = default;
    Sphere(const Point3& center, float radius, std::shared_ptr<materials::Material> material)
        : center_(center), radius_(radius), material_(material) {}

    bool hit(const core::Ray& ray, float t_min, float t_max, HitRecord& rec) const override;

private:
    Point3 center_;
    float radius_;
    std::shared_ptr<materials::Material> material_;
};

} // namespace geometry
} // namespace raytracer
