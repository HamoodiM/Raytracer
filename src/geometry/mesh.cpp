#include "mesh.h"

namespace raytracer {
namespace geometry {

Mesh::Mesh(const std::vector<Point3>& vertices, const std::vector<std::array<int, 3>>& faces,
           std::shared_ptr<materials::Material> material)
    : vertices_(vertices), faces_(faces), material_(material) {}

bool Mesh::hit(const core::Ray& ray, float t_min, float t_max, HitRecord& rec) const {
    // TODO: Implement mesh intersection
    return false;
}

} // namespace geometry
} // namespace raytracer
