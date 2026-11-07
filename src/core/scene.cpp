#include "scene.h"

namespace raytracer {
namespace core {

void Scene::add(std::shared_ptr<geometry::Primitive> object) {
    objects_.push_back(object);
    bvh_ready_ = false;
}

void Scene::add_light(std::shared_ptr<Light> light) {
    lights_.push_back(light);
}

void Scene::clear() {
    objects_.clear();
    unbounded_.clear();
    lights_.clear();
    bvh_ = acceleration::BVH();
    bvh_ready_ = false;
}

void Scene::build_acceleration(acceleration::BVH::SplitMethod method) {
    std::vector<std::shared_ptr<geometry::Primitive>> bounded;
    unbounded_.clear();
    for (const auto& object : objects_) {
        acceleration::AABB box;
        if (object->bounding_box(box))
            bounded.push_back(object);
        else
            unbounded_.push_back(object);
    }
    bvh_.build(bounded, method);
    bvh_ready_ = true;
}

bool Scene::hit(const Ray& ray, float t_min, float t_max, geometry::HitRecord& rec) const {
    if (!use_bvh_ || !bvh_ready_)
        return hit_brute_force(ray, t_min, t_max, rec);

    bool hit_anything = false;
    float closest_so_far = t_max;
    for (const auto& object : unbounded_) {
        if (object->hit(ray, t_min, closest_so_far, rec)) {
            hit_anything = true;
            closest_so_far = rec.t;
        }
    }
    if (bvh_.hit(ray, t_min, closest_so_far, rec))
        hit_anything = true;
    return hit_anything;
}

bool Scene::hit_brute_force(const Ray& ray, float t_min, float t_max, geometry::HitRecord& rec) const {
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
