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
    
    /**
     * @brief Computes UV coordinates for planar mapping
     * 
     * @param point Hit point on the plane
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
