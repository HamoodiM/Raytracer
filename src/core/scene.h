#pragma once

#include "../geometry/primitive.h"
#include <vector>
#include <memory>

namespace raytracer {
namespace core {

class Scene {
public:
    Scene() = default;

    void add(std::shared_ptr<geometry::Primitive> object);
    void clear();
    
    bool hit(const Ray& ray, float t_min, float t_max, geometry::HitRecord& rec) const;

private:
    std::vector<std::shared_ptr<geometry::Primitive>> objects_;
};

} // namespace core
} // namespace raytracer
