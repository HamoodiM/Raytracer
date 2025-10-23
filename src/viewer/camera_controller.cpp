/**
 * @file camera_controller.cpp
 * @brief Implementation of interactive camera controller
 */

#include "camera_controller.h"
#include "../common.h"
#include <glm/glm.hpp>

namespace raytracer {
namespace viewer {

CameraController::CameraController(std::shared_ptr<core::Camera> camera, 
                                 std::shared_ptr<Controller> controller)
    : camera_(camera), controller_(controller) {
}

void CameraController::update(float delta_time) {
    handle_keyboard_input(delta_time);
    handle_mouse_input(delta_time);
}

void CameraController::reset_camera() {
    camera_->update(initial_position_, initial_lookat_, initial_vup_);
}

void CameraController::set_initial_state(const Point3& position, const Point3& lookat, const Vec3& vup) {
    initial_position_ = position;
    initial_lookat_ = lookat;
    initial_vup_ = vup;
}

void CameraController::handle_keyboard_input(float delta_time) {
    float speed = controller_->movement_speed() * delta_time;
    Vec3 movement(0, 0, 0);
    
    // WASD movement
    if (controller_->is_key_pressed(GLFW_KEY_W)) {
        // Move forward
        Vec3 forward = glm::normalize(camera_->look_at() - camera_->position());
        movement += forward * speed;
    }
    if (controller_->is_key_pressed(GLFW_KEY_S)) {
        // Move backward
        Vec3 forward = glm::normalize(camera_->look_at() - camera_->position());
        movement -= forward * speed;
    }
    if (controller_->is_key_pressed(GLFW_KEY_A)) {
        // Strafe left
        Vec3 forward = glm::normalize(camera_->look_at() - camera_->position());
        Vec3 right = glm::normalize(glm::cross(forward, Vec3(0, 1, 0)));
        movement -= right * speed;
    }
    if (controller_->is_key_pressed(GLFW_KEY_D)) {
        // Strafe right
        Vec3 forward = glm::normalize(camera_->look_at() - camera_->position());
        Vec3 right = glm::normalize(glm::cross(forward, Vec3(0, 1, 0)));
        movement += right * speed;
    }
    
    // QE vertical movement
    if (controller_->is_key_pressed(GLFW_KEY_Q)) {
        movement.y -= speed;
    }
    if (controller_->is_key_pressed(GLFW_KEY_E)) {
        movement.y += speed;
    }
    
    // Apply movement
    if (glm::length(movement) > 0) {
        camera_->translate(movement);
    }
    
    // Reset camera
    if (controller_->is_key_pressed(GLFW_KEY_R)) {
        reset_camera();
    }
}

void CameraController::handle_mouse_input(float delta_time) {
    // Only rotate when right mouse button is pressed
    if (controller_->is_mouse_button_pressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        double dx, dy;
        controller_->get_mouse_delta(dx, dy);
        
        float sensitivity = controller_->mouse_sensitivity();
        float yaw = static_cast<float>(dx) * sensitivity;
        float pitch = static_cast<float>(dy) * sensitivity;
        
        camera_->rotate(yaw, pitch);
    }
    
    // Mouse wheel for FOV
    // Note: This would require additional GLFW scroll callback implementation
    // For now, we'll skip this feature
}

} // namespace viewer
} // namespace raytracer
