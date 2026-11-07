/**
 * @file mesh_normals.h
 * @brief Per-vertex normals for smooth-shaded triangle meshes
 */

#pragma once

#include "../common.h"
#include <array>
#include <vector>

namespace raytracer {
namespace geometry {

/**
 * @brief Area-weighted vertex normals
 *
 * Each face adds cross(v1 - v0, v2 - v0) to its three vertices. That vector
 * is the face normal scaled by twice the face area, so large faces count
 * more than small ones; the sums are then normalised. Orientation follows
 * the winding order. A vertex used by no face (or only by degenerate faces)
 * gets a zero vector.
 */
std::vector<Vec3> compute_vertex_normals(const std::vector<Point3>& vertices,
                                         const std::vector<std::array<int, 3>>& faces);

} // namespace geometry
} // namespace raytracer
