/**
 * @file scene_loader.cpp
 * @brief Implementation of JSON scene file parser and loader
 */

#include "scene_loader.h"
#include "../geometry/sphere.h"
#include "../geometry/plane.h"
#include "../geometry/triangle.h"
#include "../materials/lambertian.h"
#include "../materials/textured_lambertian.h"
#include "../materials/metal.h"
#include "../materials/dielectric.h"
#include "../materials/emissive.h"
#include "../textures/image_texture.h"
#include "../textures/checker_texture.h"
#include "../textures/normal_map.h"
#include "../common.h"
#include <fstream>
#include <iostream>
#include <stdexcept>

// JSON library is included in the header

namespace raytracer {
namespace core {

Scene SceneLoader::load_scene(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open scene file: " + filename);
    }
    
    nlohmann::json scene_json;
    try {
        file >> scene_json;
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error("Failed to parse JSON: " + std::string(e.what()));
    }
    
    Scene scene;
    
    // Load objects
    if (scene_json.contains("objects")) {
        for (const auto& object_json : scene_json["objects"]) {
            auto primitive = create_primitive(object_json);
            scene.add(primitive);
        }
    }
    
    // Load lights
    if (scene_json.contains("lights")) {
        for (const auto& light_json : scene_json["lights"]) {
            auto light = create_light(light_json);
            scene.add_light(light);
        }
    }
    
    return scene;
}

Camera SceneLoader::load_camera(const nlohmann::json& camera_json) {
    Point3 position = parse_vec3(camera_json["position"]);
    Point3 look_at = parse_vec3(camera_json["look_at"]);
    Vec3 vup = parse_vec3(camera_json.value("vup", nlohmann::json{0, 1, 0}));
    float vfov = camera_json.value("vfov", 40.0f);
    float aspect_ratio = camera_json.value("aspect_ratio", 16.0f/9.0f);
    float aperture = camera_json.value("aperture", 0.0f);
    float focus_distance = camera_json.value("focus_distance", 1.0f);
    
    return Camera(position, look_at, vup, vfov, aspect_ratio, aperture, focus_distance);
}

std::shared_ptr<geometry::Primitive> SceneLoader::create_primitive(const nlohmann::json& object_json) {
    std::string type = object_json["type"];
    auto material = create_material(object_json["material"]);
    
    if (type == "sphere") {
        Point3 center = parse_vec3(object_json["center"]);
        float radius = object_json["radius"];
        return std::make_shared<geometry::Sphere>(center, radius, material);
    }
    else if (type == "plane") {
        Point3 point = parse_vec3(object_json["point"]);
        Vec3 normal = parse_vec3(object_json["normal"]);
        return std::make_shared<geometry::Plane>(point, normal, material);
    }
    else if (type == "triangle") {
        Point3 v0 = parse_vec3(object_json["v0"]);
        Point3 v1 = parse_vec3(object_json["v1"]);
        Point3 v2 = parse_vec3(object_json["v2"]);
        return std::make_shared<geometry::Triangle>(v0, v1, v2, material);
    }
    else {
        throw std::runtime_error("Unknown primitive type: " + type);
    }
}

std::shared_ptr<materials::Material> SceneLoader::create_material(const nlohmann::json& material_json) {
    std::string type = material_json["type"];
    
    if (type == "lambertian") {
        if (material_json.contains("texture")) {
            auto texture = create_texture(material_json["texture"]);
            return std::make_shared<materials::Lambertian>(texture);
        } else {
            Color albedo = parse_color(material_json["albedo"]);
            return std::make_shared<materials::Lambertian>(albedo);
        }
    }
    else if (type == "textured_lambertian") {
        auto albedo_texture = create_texture(material_json["albedo_texture"]);
        std::shared_ptr<textures::NormalMap> normal_map = nullptr;
        if (material_json.contains("normal_map")) {
            auto normal_texture = create_texture(material_json["normal_map"]);
            float strength = material_json.value("normal_strength", 1.0f);
            normal_map = std::make_shared<textures::NormalMap>(normal_texture, strength);
        }
        return std::make_shared<materials::TexturedLambertian>(albedo_texture, normal_map);
    }
    else if (type == "metal") {
        Color albedo = parse_color(material_json["albedo"]);
        float fuzz = material_json.value("fuzz", 0.0f);
        return std::make_shared<materials::Metal>(albedo, fuzz);
    }
    else if (type == "dielectric") {
        float ir = material_json.value("ir", 1.5f);
        return std::make_shared<materials::Dielectric>(ir);
    }
    else if (type == "emissive") {
        Color emission = parse_color(material_json["emission"]);
        return std::make_shared<materials::Emissive>(emission);
    }
    else {
        throw std::runtime_error("Unknown material type: " + type);
    }
}

std::shared_ptr<textures::Texture> SceneLoader::create_texture(const nlohmann::json& texture_json) {
    if (texture_json.is_string()) {
        // Image texture from filename
        std::string filename = texture_json;
        return std::make_shared<textures::ImageTexture>(filename);
    }
    else if (texture_json.is_object()) {
        std::string type = texture_json["type"];
        
        if (type == "checker") {
            Color color1 = parse_color(texture_json["color1"]);
            Color color2 = parse_color(texture_json["color2"]);
            float frequency = texture_json.value("frequency", 10.0f);
            return std::make_shared<textures::CheckerTexture>(color1, color2, frequency);
        }
        else {
            throw std::runtime_error("Unknown texture type: " + type);
        }
    }
    else {
        throw std::runtime_error("Invalid texture configuration");
    }
}

std::shared_ptr<Light> SceneLoader::create_light(const nlohmann::json& light_json) {
    std::string type = light_json["type"];
    
    if (type == "point") {
        Point3 position = parse_vec3(light_json["position"]);
        Color intensity = parse_color(light_json["intensity"]);
        return std::make_shared<PointLight>(position, intensity);
    }
    else if (type == "area") {
        Point3 center = parse_vec3(light_json["center"]);
        Vec3 u = parse_vec3(light_json["u"]);
        Vec3 v = parse_vec3(light_json["v"]);
        Color emission = parse_color(light_json["emission"]);
        return std::make_shared<AreaLight>(center, u, v, emission);
    }
    else {
        throw std::runtime_error("Unknown light type: " + type);
    }
}

Vec3 SceneLoader::parse_vec3(const nlohmann::json& json_array) {
    if (!json_array.is_array() || json_array.size() != 3) {
        throw std::runtime_error("Invalid Vec3 format - expected [x, y, z] array");
    }
    
    return Vec3(json_array[0], json_array[1], json_array[2]);
}

Color SceneLoader::parse_color(const nlohmann::json& json_array) {
    if (!json_array.is_array() || json_array.size() != 3) {
        throw std::runtime_error("Invalid Color format - expected [r, g, b] array");
    }
    
    return Color(json_array[0], json_array[1], json_array[2]);
}

} // namespace core
} // namespace raytracer
