#pragma once

#include "material.h"

namespace raytracer {
namespace materials {

class Emissive : public Material {
public:
    Emissive(const Color& emit) : emit_(emit) {}

    bool scatter(const core::Ray& ray_in, const geometry::HitRecord& rec, 
                Color& attenuation, core::Ray& scattered) const override;

private:
    Color emit_;
};

} // namespace materials
} // namespace raytracer
