#pragma once

#include "material.h"
#include "../textures/texture.h"
#include <memory>

namespace raytracer {
namespace materials {

class Lambertian : public Material {
public:
    /**
     * @brief Constructs a Lambertian material with a solid color
     * 
     * @param albedo Diffuse color of the material
     */
    Lambertian(const Color& albedo);
    
    /**
     * @brief Constructs a Lambertian material with a texture
     * 
     * @param texture Texture to use for albedo sampling
     */
    Lambertian(std::shared_ptr<textures::Texture> texture);

    bool scatter(const core::Ray& ray_in, const geometry::HitRecord& rec, 
                Color& attenuation, core::Ray& scattered) const override;

private:
    std::shared_ptr<textures::Texture> texture_;
    Color solid_albedo_;
    bool use_texture_;
};

} // namespace materials
} // namespace raytracer
