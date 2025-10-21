#pragma once

#include "material.h"

namespace raytracer {
namespace materials {

class Lambertian : public Material {
public:
    Lambertian(const Color& albedo) : albedo_(albedo) {}

    bool scatter(const core::Ray& ray_in, const geometry::HitRecord& rec, 
                Color& attenuation, core::Ray& scattered) const override;

private:
    Color albedo_;
};

} // namespace materials
} // namespace raytracer
