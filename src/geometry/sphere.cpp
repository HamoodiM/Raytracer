#include "sphere.h"
#include "../common.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace raytracer {
namespace geometry {

bool Sphere::hit(const core::Ray& ray, float t_min, float t_max, HitRecord& rec) const {
    Vec3 oc = ray.origin() - center_;
    auto a = glm::dot(ray.direction(), ray.direction());
    auto half_b = glm::dot(oc, ray.direction());
    auto c = glm::dot(oc, oc) - radius_ * radius_;

    auto discriminant = half_b * half_b - a * c;
    if (discriminant < 0) return false;

    // Find the nearest root that lies in the acceptable range
    auto sqrtd = sqrt(discriminant);
    auto root = (-half_b - sqrtd) / a;
    if (root < t_min || t_max < root) {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || t_max < root)
            return false;
    }

    rec.t = root;
    rec.point = ray.at(rec.t);
    Vec3 outward_normal = (rec.point - center_) / radius_;
    rec.set_face_normal(ray, outward_normal);
    rec.material = material_;
    
    // Compute UV coordinates for spherical mapping
    compute_uv(rec.point, rec.u, rec.v);
    
    // Compute tangent space vectors for normal mapping
    compute_tangent_space(outward_normal, rec);

    return true;
}

void Sphere::compute_uv(const Point3& point, float& u, float& v) const {
    // Convert hit point to local coordinates (relative to sphere center)
    Vec3 local_point = point - center_;
    
    // Normalize to get unit vector
    Vec3 unit_point = glm::normalize(local_point);
    
    // Compute spherical coordinates
    // phi: azimuthal angle (0 to 2π)
    // theta: polar angle (0 to π)
    float phi = std::atan2(unit_point.z, unit_point.x);
    float theta = std::acos(unit_point.y);
    
    // Convert to UV coordinates [0, 1]
    u = 1.0f - (phi + glm::pi<float>()) / (2.0f * glm::pi<float>());
    v = theta / glm::pi<float>();
}

void Sphere::compute_tangent_space(const Vec3& normal, HitRecord& rec) const {
    // For a sphere, we can compute tangent and bitangent from the normal
    // Choose an arbitrary vector perpendicular to the normal
    Vec3 up(0, 1, 0);
    if (std::abs(glm::dot(normal, up)) > 0.9f) {
        up = Vec3(1, 0, 0); // Use different reference if normal is nearly parallel to up
    }
    
    // Compute tangent using cross product
    Vec3 tangent = glm::normalize(glm::cross(up, normal));
    
    // Compute bitangent using cross product
    Vec3 bitangent = glm::normalize(glm::cross(normal, tangent));
    
    rec.set_tangent_space(tangent, bitangent);
}

} // namespace geometry
} // namespace raytracer
