#pragma once

#include "material.h"

namespace raytracer {
namespace materials {

class Dielectric : public Material {
public:
    Dielectric(float index_of_refraction) : ir_(index_of_refraction) {}

    bool scatter(const core::Ray& ray_in, const geometry::HitRecord& rec, 
                Color& attenuation, core::Ray& scattered) const override;

private:
    float ir_; // Index of Refraction
};

} // namespace materials
} // namespace raytracer
