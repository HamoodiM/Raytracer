#pragma once

#include "../geometry/primitive.h"
#include "light.h"
#include <vector>
#include <memory>

namespace raytracer {
namespace core {

class Scene {
public:
    Scene() = default;

    void add(std::shared_ptr<geometry::Primitive> object);
    void add_light(std::shared_ptr<Light> light);
    void clear();
    
    bool hit(const Ray& ray, float t_min, float t_max, geometry::HitRecord& rec) const;
    const std::vector<std::shared_ptr<Light>>& lights() const { return lights_; }

private:
    std::vector<std::shared_ptr<geometry::Primitive>> objects_;
    std::vector<std::shared_ptr<Light>> lights_;
};

} // namespace core
} // namespace raytracer
