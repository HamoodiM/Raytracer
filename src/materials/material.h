#pragma once

#include "../common.h"
#include "../core/ray.h"

namespace raytracer {
namespace geometry {
    struct HitRecord;
}

namespace materials {

class Material {
public:
    virtual ~Material() = default;
    virtual bool scatter(const core::Ray& ray_in, const geometry::HitRecord& rec, 
                        Color& attenuation, core::Ray& scattered) const = 0;
    virtual Color emit() const { return Color(0, 0, 0); }
};

} // namespace materials
} // namespace raytracer
