/**
 * @file scene_gallery.h
 * @brief Scene gallery manager for loading and managing demo scenes
 * 
 * This class provides a centralized way to load and manage different
 * demo scenes that showcase various features of the raytracer.
 */

#pragma once

#include "scene.h"
#include "camera.h"
#include <string>
#include <vector>
#include <memory>

namespace raytracer {
namespace core {

struct SceneInfo {
    std::string name;
    std::string description;
    std::string filename;
    std::string features;
};

class SceneGallery {
public:
    /**
     * @brief Gets the list of available scenes
     * 
     * @return Vector of scene information
     */
    static const std::vector<SceneInfo>& get_scenes();
    
    /**
     * @brief Loads a scene by index
     * 
     * @param index Scene index in the gallery
     * @return Pair of (scene, camera) objects
     * @throws std::out_of_range if index is invalid
     */
    static std::pair<Scene, Camera> load_scene(int index);
    
    /**
     * @brief Loads a scene by name
     * 
     * @param name Scene name
     * @return Pair of (scene, camera) objects
     * @throws std::runtime_error if scene not found
     */
    static std::pair<Scene, Camera> load_scene(const std::string& name);
    
    /**
     * @brief Gets the number of available scenes
     * 
     * @return Number of scenes in the gallery
     */
    static size_t scene_count();

private:
    static std::vector<SceneInfo> scenes_;
    static bool initialized_;
    
    /**
     * @brief Initializes the scene gallery with available scenes
     */
    static void initialize();
};

} // namespace core
} // namespace raytracer
