/**
 * @file scene_gallery.cpp
 * @brief Implementation of scene gallery manager
 */

#include "scene_gallery.h"
#include "scene_loader.h"
#include <stdexcept>
#include <iostream>

namespace raytracer {
namespace core {

std::vector<SceneInfo> SceneGallery::scenes_;
bool SceneGallery::initialized_ = false;

const std::vector<SceneInfo>& SceneGallery::get_scenes() {
    if (!initialized_) {
        initialize();
    }
    return scenes_;
}

std::pair<Scene, Camera> SceneGallery::load_scene(int index) {
    if (!initialized_) {
        initialize();
    }
    
    if (index < 0 || index >= static_cast<int>(scenes_.size())) {
        throw std::out_of_range("Scene index out of range: " + std::to_string(index));
    }
    
    const auto& scene_info = scenes_[index];
    std::cout << "Loading scene: " << scene_info.name << std::endl;
    std::cout << "Description: " << scene_info.description << std::endl;
    std::cout << "Features: " << scene_info.features << std::endl;
    
    // Load scene and camera from JSON
    Scene scene = SceneLoader::load_scene(scene_info.filename);
    
    // For now, we'll create a default camera since the JSON format
    // doesn't include camera configuration in the current implementation
    // In a full implementation, you would load camera from JSON as well
    Camera camera(Point3(0, 0, 5), Point3(0, 0, 0), Vec3(0, 1, 0), 40, 1.333f);
    
    return std::make_pair(std::move(scene), std::move(camera));
}

std::pair<Scene, Camera> SceneGallery::load_scene(const std::string& name) {
    if (!initialized_) {
        initialize();
    }
    
    for (size_t i = 0; i < scenes_.size(); ++i) {
        if (scenes_[i].name == name) {
            return load_scene(static_cast<int>(i));
        }
    }
    
    throw std::runtime_error("Scene not found: " + name);
}

size_t SceneGallery::scene_count() {
    if (!initialized_) {
        initialize();
    }
    return scenes_.size();
}

void SceneGallery::initialize() {
    scenes_ = {
        {
            "Cornell Box",
            "Classic Cornell box with diffuse and metallic spheres",
            "scenes/cornell_box.json",
            "Basic materials, area lighting, Cornell box setup"
        },
        {
            "Textured Spheres",
            "Spheres with procedural checker textures and depth of field",
            "scenes/textured_spheres.json",
            "Texture mapping, procedural textures, depth of field"
        },
        {
            "Glass Showcase",
            "Dielectric spheres with different refractive indices",
            "scenes/glass_showcase.json",
            "Dielectric materials, refraction, depth of field"
        },
        {
            "Material Test",
            "All material types in one scene for comparison",
            "scenes/material_test.json",
            "All material types: Lambertian, Metal, Dielectric, Emissive"
        },
        {
            "Normal Map Demo",
            "Spheres with checker patterns demonstrating surface detail",
            "scenes/normal_map_demo.json",
            "Texture mapping, surface detail, multi-light setup"
        }
    };
    
    initialized_ = true;
}

} // namespace core
} // namespace raytracer
