#pragma once

#include <GLFW/glfw3.h>
#include "../rendering/framebuffer.h"
#include <memory>

namespace raytracer {
namespace viewer {

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();
    
    bool should_close() const;
    void poll_events();
    void swap_buffers();
    
    void display_framebuffer(const rendering::Framebuffer& framebuffer);
    
    GLFWwindow* get_handle() const { return window_; }
    int width() const { return width_; }
    int height() const { return height_; }

private:
    GLFWwindow* window_;
    int width_, height_;
    unsigned int texture_;
    unsigned int VAO_, VBO_;
    
    void setup_opengl();
    void create_quad();
};

} // namespace viewer
} // namespace raytracer
