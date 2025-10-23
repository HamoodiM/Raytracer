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
    
    // UV coordinates for texture mapping
    float u, v;
    
    // Tangent space vectors for normal mapping
    Vec3 tangent;
    Vec3 bitangent;

    inline void set_face_normal(const core::Ray& ray, const Vec3& outward_normal) {
        front_face = glm::dot(ray.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
    
    /**
     * @brief Sets the tangent space vectors for normal mapping
     * 
     * @param t Tangent vector (should be normalized)
     * @param b Bitangent vector (should be normalized)
     */
    inline void set_tangent_space(const Vec3& t, const Vec3& b) {
        tangent = t;
        bitangent = b;
    }
};

class Primitive {
public:
    virtual ~Primitive() = default;
    virtual bool hit(const core::Ray& ray, float t_min, float t_max, HitRecord& rec) const = 0;
};

} // namespace geometry
} // namespace raytracer
