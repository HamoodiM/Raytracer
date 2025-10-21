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

private:
    GLFWwindow* window_;
    double last_mouse_x_, last_mouse_y_;
    double mouse_dx_, mouse_dy_;
    bool first_mouse_;
    
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
};

} // namespace viewer
} // namespace raytracer
