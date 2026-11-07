/**
 * @file mesh_normals.cpp
 * @brief Area-weighted vertex normals
 */

#include "mesh_normals.h"
#include <glm/glm.hpp>

namespace raytracer {
namespace geometry {

std::vector<Vec3> compute_vertex_normals(const std::vector<Point3>& vertices,
                                         const std::vector<std::array<int, 3>>& faces) {
    std::vector<Vec3> normals(vertices.size(), Vec3(0.0f));
    for (const auto& f : faces) {
        const Point3& a = vertices[f[0]];
        const Point3& b = vertices[f[1]];
        const Point3& c = vertices[f[2]];
        Vec3 weighted = glm::cross(b - a, c - a);  // |weighted| = 2 * area
        normals[f[0]] += weighted;
        normals[f[1]] += weighted;
        normals[f[2]] += weighted;
    }
    for (auto& n : normals) {
        float len = glm::length(n);
        if (len > 0.0f) n /= len;
    }
    return normals;
}

} // namespace geometry
} // namespace raytracer
