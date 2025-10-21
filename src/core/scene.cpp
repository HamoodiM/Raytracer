#include "scene.h"

namespace raytracer {
namespace core {

void Scene::add(std::shared_ptr<geometry::Primitive> object) {
    objects_.push_back(object);
}

void Scene::clear() {
    objects_.clear();
}

bool Scene::hit(const Ray& ray, float t_min, float t_max, geometry::HitRecord& rec) const {
    geometry::HitRecord temp_rec;
    bool hit_anything = false;
    auto closest_so_far = t_max;

    for (const auto& object : objects_) {
        if (object->hit(ray, t_min, closest_so_far, temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }

    return hit_anything;
}

} // namespace core
} // namespace raytracer
