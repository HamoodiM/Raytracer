#pragma once

#include "material.h"

namespace raytracer {
namespace materials {

class Metal : public Material {
public:
    Metal(const Color& albedo, float fuzz) : albedo_(albedo), fuzz_(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const core::Ray& ray_in, const geometry::HitRecord& rec, 
                Color& attenuation, core::Ray& scattered) const override;

private:
    Color albedo_;
    float fuzz_;
};

} // namespace materials
} // namespace raytracer
