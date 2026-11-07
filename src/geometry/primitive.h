#pragma once

#include "../common.h"
#include "../core/ray.h"
#include "../materials/material.h"
#include "../acceleration/aabb.h"
#include <memory>

namespace raytracer {
namespace geometry {

struct HitRecord {
    Point3 point;
    // Shading normal (faces the incoming ray). Equal to geometric_normal
    // except on smooth-shaded triangles, where it is interpolated from the
    // vertex normals.
    Vec3 normal;
    // True surface normal (faces the incoming ray). Use it for front/back
    // decisions and for "is this direction above the surface" tests.
    Vec3 geometric_normal;
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
        geometric_normal = normal;
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

    /**
     * @brief World-space bounding box, used to build the BVH
     *
     * @return false for unbounded primitives (e.g. infinite planes), which
     *         the scene then tests separately, outside the BVH
     */
    virtual bool bounding_box(acceleration::AABB& box) const {
        (void)box;
        return false;
    }
};

} // namespace geometry
} // namespace raytracer
