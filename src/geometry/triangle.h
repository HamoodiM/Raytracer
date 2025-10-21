#pragma once

#include "primitive.h"
#include "../materials/material.h"
#include <memory>

namespace raytracer {
namespace geometry {

class Triangle : public Primitive {
public:
    Triangle() = default;
    Triangle(const Point3& v0, const Point3& v1, const Point3& v2, 
             std::shared_ptr<materials::Material> material)
        : v0_(v0), v1_(v1), v2_(v2), material_(material) {}

    bool hit(const core::Ray& ray, float t_min, float t_max, HitRecord& rec) const override;

private:
    Point3 v0_, v1_, v2_;
    std::shared_ptr<materials::Material> material_;
};

} // namespace geometry
} // namespace raytracer
