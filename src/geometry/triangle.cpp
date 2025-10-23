#include "triangle.h"
#include "../common.h"
#include <glm/glm.hpp>

namespace raytracer {
namespace geometry {

bool Triangle::hit(const core::Ray& ray, float t_min, float t_max, HitRecord& rec) const {
    // Möller-Trumbore algorithm
    Vec3 edge1 = v1_ - v0_;
    Vec3 edge2 = v2_ - v0_;
    Vec3 h = glm::cross(ray.direction(), edge2);
    float a = glm::dot(edge1, h);
    
    // Ray is parallel to triangle
    if (a > -1e-8f && a < 1e-8f) {
        return false;
    }
    
    float f = 1.0f / a;
    Vec3 s = ray.origin() - v0_;
    float u = f * glm::dot(s, h);
    
    if (u < 0.0f || u > 1.0f) {
        return false;
    }
    
    Vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(ray.direction(), q);
    
    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }
    
    float t = f * glm::dot(edge2, q);
    
    if (t < t_min || t > t_max) {
        return false;
    }
    
    rec.t = t;
    rec.point = ray.at(rec.t);
    
    // Compute normal using cross product
    Vec3 outward_normal = glm::normalize(glm::cross(edge1, edge2));
    rec.set_face_normal(ray, outward_normal);
    rec.material = material_;
    
    // Compute UV coordinates using barycentric coordinates
    compute_uv(u, v, rec.u, rec.v);
    
    // Compute tangent space vectors for normal mapping
    compute_tangent_space(outward_normal, rec);
    
    return true;
}

void Triangle::compute_uv(float u, float v, float& out_u, float& out_v) const {
    // For now, use barycentric coordinates directly as UV coordinates
    // In a more advanced implementation, you would store UV coordinates per vertex
    // and interpolate them using barycentric coordinates
    out_u = u;
    out_v = v;
}

void Triangle::compute_tangent_space(const Vec3& normal, HitRecord& rec) const {
    // For triangles, we can compute tangent from the edge vectors
    Vec3 edge1 = v1_ - v0_;
    Vec3 edge2 = v2_ - v0_;
    
    // Use edge1 as tangent direction
    Vec3 tangent = glm::normalize(edge1);
    
    // Compute bitangent using cross product
    Vec3 bitangent = glm::normalize(glm::cross(normal, tangent));
    
    rec.set_tangent_space(tangent, bitangent);
}

} // namespace geometry
} // namespace raytracer
