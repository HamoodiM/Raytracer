#pragma once

#include "primitive.h"
#include "../materials/material.h"
#include <memory>
#include <vector>

namespace raytracer {
namespace geometry {

class Mesh : public Primitive {
public:
    Mesh() = default;
    Mesh(const std::vector<Point3>& vertices, const std::vector<std::array<int, 3>>& faces,
         std::shared_ptr<materials::Material> material);

    bool hit(const core::Ray& ray, float t_min, float t_max, HitRecord& rec) const override;

private:
    std::vector<Point3> vertices_;
    std::vector<std::array<int, 3>> faces_;
    std::shared_ptr<materials::Material> material_;
};

} // namespace geometry
} // namespace raytracer
