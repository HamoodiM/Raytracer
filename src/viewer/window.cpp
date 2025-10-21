#include "window.h"
#include <iostream>

namespace raytracer {
namespace viewer {

Window::Window(int width, int height, const std::string& title) 
    : width_(width), height_(height) {
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }
    
    // Create window (no OpenGL context for now)
    window_ = glfwCreateWindow(width_, height_, title.c_str(), nullptr, nullptr);
    if (!window_) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    
    // Initialize member variables
    texture_ = 0;
    VAO_ = 0;
    VBO_ = 0;
}

Window::~Window() {
    if (window_) {
        glfwDestroyWindow(window_);
    }
    glfwTerminate();
}

bool Window::should_close() const {
    return glfwWindowShouldClose(window_);
}

void Window::poll_events() {
    glfwPollEvents();
}

void Window::swap_buffers() {
    glfwSwapBuffers(window_);
}

void Window::display_framebuffer(const rendering::Framebuffer& framebuffer) {
    // For now, just print a message - we'll implement proper display later
    std::cout << "Displaying framebuffer (" << framebuffer.width() << "x" << framebuffer.height() << ")" << std::endl;
    std::cout << "Image data available at: " << framebuffer.data() << std::endl;
    
    // Show some sample pixel colors
    if (framebuffer.width() > 0 && framebuffer.height() > 0) {
        auto center_color = framebuffer.get_pixel(framebuffer.width()/2, framebuffer.height()/2);
        std::cout << "Center pixel color: (" << center_color.r << ", " << center_color.g << ", " << center_color.b << ")" << std::endl;
    }
}

void Window::setup_opengl() {
    // Placeholder for OpenGL setup
}

void Window::create_quad() {
    // Placeholder for quad creation
}

} // namespace viewer
} // namespace raytracer
