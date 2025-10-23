#include "plane.h"
#include "../common.h"
#include <glm/glm.hpp>

namespace raytracer {
namespace geometry {

bool Plane::hit(const core::Ray& ray, float t_min, float t_max, HitRecord& rec) const {
    Vec3 normalized_normal = glm::normalize(normal_);
    float denominator = glm::dot(ray.direction(), normalized_normal);
    
    // Ray is parallel to plane
    if (std::abs(denominator) < 1e-8f) {
        return false;
    }
    
    Vec3 oc = point_ - ray.origin();
    float t = glm::dot(oc, normalized_normal) / denominator;
    
    if (t < t_min || t > t_max) {
        return false;
    }
    
    rec.t = t;
    rec.point = ray.at(rec.t);
    rec.set_face_normal(ray, normalized_normal);
    rec.material = material_;
    
    // Compute UV coordinates for planar mapping
    compute_uv(rec.point, rec.u, rec.v);
    
    // Compute tangent space vectors for normal mapping
    compute_tangent_space(normalized_normal, rec);
    
    return true;
}

void Plane::compute_uv(const Point3& point, float& u, float& v) const {
    // Create a coordinate system on the plane
    Vec3 normalized_normal = glm::normalize(normal_);
    
    // Choose arbitrary vectors for UV mapping
    Vec3 u_axis, v_axis;
    
    // Find a vector perpendicular to the normal
    if (std::abs(normalized_normal.x) < 0.9f) {
        u_axis = glm::normalize(glm::cross(normalized_normal, Vec3(1, 0, 0)));
    } else {
        u_axis = glm::normalize(glm::cross(normalized_normal, Vec3(0, 1, 0)));
    }
    
    v_axis = glm::normalize(glm::cross(normalized_normal, u_axis));
    
    // Project point onto plane and compute UV coordinates
    Vec3 local_point = point - point_;
    u = glm::dot(local_point, u_axis) * 0.1f; // Scale factor for texture size
    v = glm::dot(local_point, v_axis) * 0.1f;
    
    // Wrap coordinates to [0, 1] range
    u = u - std::floor(u);
    v = v - std::floor(v);
}

void Plane::compute_tangent_space(const Vec3& normal, HitRecord& rec) const {
    // For a plane, we can use the same vectors as in UV computation
    Vec3 normalized_normal = glm::normalize(normal);
    
    // Find tangent vector
    Vec3 tangent;
    if (std::abs(normalized_normal.x) < 0.9f) {
        tangent = glm::normalize(glm::cross(normalized_normal, Vec3(1, 0, 0)));
    } else {
        tangent = glm::normalize(glm::cross(normalized_normal, Vec3(0, 1, 0)));
    }
    
    // Compute bitangent
    Vec3 bitangent = glm::normalize(glm::cross(normalized_normal, tangent));
    
    rec.set_tangent_space(tangent, bitangent);
}

} // namespace geometry
} // namespace raytracer
