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

    /**
     * @brief Triangle with per-vertex normals (smooth shading)
     *
     * The shading normal is interpolated from n0, n1, n2 with the hit's
     * barycentric coordinates; the geometric (face) normal is still used for
     * front/back-face decisions.
     */
    Triangle(const Point3& v0, const Point3& v1, const Point3& v2,
             const Vec3& n0, const Vec3& n1, const Vec3& n2,
             std::shared_ptr<materials::Material> material)
        : v0_(v0), v1_(v1), v2_(v2), n0_(n0), n1_(n1), n2_(n2), smooth_(true), material_(material) {}

    bool hit(const core::Ray& ray, float t_min, float t_max, HitRecord& rec) const override;
    bool bounding_box(acceleration::AABB& box) const override;

private:
    Point3 v0_, v1_, v2_;
    Vec3 n0_{0.0f}, n1_{0.0f}, n2_{0.0f};  // vertex normals, used when smooth_
    bool smooth_ = false;
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
