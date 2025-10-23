/**
 * @file textured_lambertian.h
 * @brief Lambertian material with texture and normal map support
 * 
 * This material extends the basic Lambertian material to support both
 * texture mapping for albedo and normal mapping for surface detail.
 */

#pragma once

#include "material.h"
#include "../textures/texture.h"
#include "../textures/normal_map.h"
#include <memory>

namespace raytracer {
namespace materials {

class TexturedLambertian : public Material {
public:
    /**
     * @brief Constructs a textured Lambertian material
     * 
     * @param albedo_texture Texture for diffuse albedo
     * @param normal_map Optional normal map for surface detail
     */
    TexturedLambertian(std::shared_ptr<textures::Texture> albedo_texture,
                      std::shared_ptr<textures::NormalMap> normal_map = nullptr);

    bool scatter(const core::Ray& ray_in, const geometry::HitRecord& rec, 
                Color& attenuation, core::Ray& scattered) const override;

private:
    std::shared_ptr<textures::Texture> albedo_texture_;
    std::shared_ptr<textures::NormalMap> normal_map_;
};

} // namespace materials
} // namespace raytracer
