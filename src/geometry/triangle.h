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
    
    /**
     * @brief Computes UV coordinates using barycentric coordinates
     * 
     * @param u Barycentric coordinate u
     * @param v Barycentric coordinate v
     * @param out_u Output U coordinate [0, 1]
     * @param out_v Output V coordinate [0, 1]
     */
    void compute_uv(float u, float v, float& out_u, float& out_v) const;
    
    /**
     * @brief Computes tangent space vectors for normal mapping
     * 
     * @param normal Surface normal at hit point
     * @param rec Hit record to store tangent and bitangent
     */
    void compute_tangent_space(const Vec3& normal, HitRecord& rec) const;
};

} // namespace geometry
} // namespace raytracer
