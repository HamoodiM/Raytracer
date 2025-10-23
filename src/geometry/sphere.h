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
    
    /**
     * @brief Computes UV coordinates for spherical mapping
     * 
     * @param point Hit point on the sphere
     * @param u Output U coordinate [0, 1]
     * @param v Output V coordinate [0, 1]
     */
    void compute_uv(const Point3& point, float& u, float& v) const;
    
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
