#include "controller.h"

namespace raytracer {
namespace viewer {

Controller::Controller(GLFWwindow* window) 
    : window_(window), last_mouse_x_(0), last_mouse_y_(0), 
      mouse_dx_(0), mouse_dy_(0), first_mouse_(true) {
    
    glfwSetWindowUserPointer(window_, this);
    glfwSetCursorPosCallback(window_, mouse_callback);
    glfwSetKeyCallback(window_, key_callback);
}

void Controller::update() {
    mouse_dx_ = 0;
    mouse_dy_ = 0;
}

bool Controller::is_key_pressed(int key) const {
    return glfwGetKey(window_, key) == GLFW_PRESS;
}

bool Controller::is_mouse_button_pressed(int button) const {
    return glfwGetMouseButton(window_, button) == GLFW_PRESS;
}

void Controller::get_mouse_position(double& x, double& y) const {
    glfwGetCursorPos(window_, &x, &y);
}

void Controller::get_mouse_delta(double& dx, double& dy) {
    dx = mouse_dx_;
    dy = mouse_dy_;
}

void Controller::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    Controller* controller = static_cast<Controller*>(glfwGetWindowUserPointer(window));
    
    if (controller->first_mouse_) {
        controller->last_mouse_x_ = xpos;
        controller->last_mouse_y_ = ypos;
        controller->first_mouse_ = false;
    }
    
    controller->mouse_dx_ = xpos - controller->last_mouse_x_;
    controller->mouse_dy_ = controller->last_mouse_y_ - ypos; // Reversed since y-coordinates go from bottom to top
    
    controller->last_mouse_x_ = xpos;
    controller->last_mouse_y_ = ypos;
}

void Controller::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Handle key events if needed
}

} // namespace viewer
} // namespace raytracer
