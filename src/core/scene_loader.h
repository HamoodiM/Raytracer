/**
 * @file scene_loader.h
 * @brief JSON scene file parser and loader
 * 
 * This class loads scene configurations from JSON files, including
 * camera settings, objects, materials, textures, and lights.
 */

#pragma once

#include "scene.h"
#include "camera.h"
#include "../geometry/primitive.h"
#include "../materials/material.h"
#include "../textures/texture.h"
#include "../textures/normal_map.h"
#include <string>
#include <memory>

// Include JSON library
#include "../../external/json.hpp"

namespace raytracer {
namespace core {

class SceneLoader {
public:
    /**
     * @brief Loads a scene from a JSON file
     * 
     * @param filename Path to the JSON scene file
     * @return Loaded scene object
     * @throws std::runtime_error if the file cannot be loaded or parsed
     */
    static Scene load_scene(const std::string& filename);
    
    /**
     * @brief Loads a camera from JSON configuration
     * 
     * @param camera_json JSON object containing camera parameters
     * @return Configured camera object
     */
    static Camera load_camera(const nlohmann::json& camera_json);

private:
    /**
     * @brief Creates a primitive object from JSON configuration
     * 
     * @param object_json JSON object containing primitive parameters
     * @return Created primitive object
     */
    static std::shared_ptr<geometry::Primitive> create_primitive(const nlohmann::json& object_json);
    
    /**
     * @brief Creates a material from JSON configuration
     * 
     * @param material_json JSON object containing material parameters
     * @return Created material object
     */
    static std::shared_ptr<materials::Material> create_material(const nlohmann::json& material_json);
    
    /**
     * @brief Creates a texture from JSON configuration
     * 
     * @param texture_json JSON object containing texture parameters
     * @return Created texture object
     */
    static std::shared_ptr<textures::Texture> create_texture(const nlohmann::json& texture_json);
    
    /**
     * @brief Creates a light from JSON configuration
     * 
     * @param light_json JSON object containing light parameters
     * @return Created light object
     */
    static std::shared_ptr<Light> create_light(const nlohmann::json& light_json);
    
    /**
     * @brief Parses a 3D vector from JSON array
     * 
     * @param json_array JSON array containing [x, y, z] values
     * @return Parsed Vec3
     */
    static Vec3 parse_vec3(const nlohmann::json& json_array);
    
    /**
     * @brief Parses a color from JSON array
     * 
     * @param json_array JSON array containing [r, g, b] values
     * @return Parsed Color
     */
    static Color parse_color(const nlohmann::json& json_array);
};

} // namespace core
} // namespace raytracer
