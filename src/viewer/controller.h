#pragma once

#include <GLFW/glfw3.h>

namespace raytracer {
namespace viewer {

class Controller {
public:
    Controller(GLFWwindow* window);
    
    void update();
    bool is_key_pressed(int key) const;
    bool is_mouse_button_pressed(int button) const;
    void get_mouse_position(double& x, double& y) const;
    void get_mouse_delta(double& dx, double& dy);
    
    /**
     * @brief Gets the movement speed for camera controls
     * 
     * @return Movement speed in units per second
     */
    float movement_speed() const { return movement_speed_; }
    
    /**
     * @brief Sets the movement speed for camera controls
     * 
     * @param speed Movement speed in units per second
     */
    void set_movement_speed(float speed) { movement_speed_ = speed; }
    
    /**
     * @brief Gets the mouse sensitivity for camera rotation
     * 
     * @return Mouse sensitivity multiplier
     */
    float mouse_sensitivity() const { return mouse_sensitivity_; }
    
    /**
     * @brief Sets the mouse sensitivity for camera rotation
     * 
     * @param sensitivity Mouse sensitivity multiplier
     */
    void set_mouse_sensitivity(float sensitivity) { mouse_sensitivity_ = sensitivity; }

private:
    GLFWwindow* window_;
    double last_mouse_x_, last_mouse_y_;
    double mouse_dx_, mouse_dy_;
    bool first_mouse_;
    float movement_speed_;
    float mouse_sensitivity_;
    
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
};

} // namespace viewer
} // namespace raytracer
