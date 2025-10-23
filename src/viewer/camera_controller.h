/**
 * @file camera_controller.h
 * @brief Interactive camera controller for real-time scene navigation
 * 
 * This class integrates input handling with camera movement to provide
 * first-person camera controls for scene exploration.
 */

#pragma once

#include "../core/camera.h"
#include "controller.h"
#include <memory>

namespace raytracer {
namespace viewer {

class CameraController {
public:
    /**
     * @brief Constructs a camera controller
     * 
     * @param camera Camera to control
     * @param controller Input controller for handling user input
     */
    CameraController(std::shared_ptr<core::Camera> camera, 
                    std::shared_ptr<Controller> controller);
    
    /**
     * @brief Updates camera based on input and frame time
     * 
     * @param delta_time Time elapsed since last frame in seconds
     */
    void update(float delta_time);
    
    /**
     * @brief Resets camera to initial position and orientation
     */
    void reset_camera();
    
    /**
     * @brief Sets the initial camera state for reset functionality
     * 
     * @param position Initial camera position
     * @param lookat Initial look-at target
     * @param vup Initial up vector
     */
    void set_initial_state(const Point3& position, const Point3& lookat, const Vec3& vup);

private:
    std::shared_ptr<core::Camera> camera_;
    std::shared_ptr<Controller> controller_;
    
    // Initial camera state for reset
    Point3 initial_position_;
    Point3 initial_lookat_;
    Vec3 initial_vup_;
    
    /**
     * @brief Handles keyboard input for camera movement
     * 
     * @param delta_time Time elapsed since last frame
     */
    void handle_keyboard_input(float delta_time);
    
    /**
     * @brief Handles mouse input for camera rotation
     * 
     * @param delta_time Time elapsed since last frame
     */
    void handle_mouse_input(float delta_time);
};

} // namespace viewer
} // namespace raytracer
