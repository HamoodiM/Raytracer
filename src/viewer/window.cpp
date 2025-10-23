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
    
    // Create window
    window_ = glfwCreateWindow(width_, height_, title.c_str(), nullptr, nullptr);
    if (!window_) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    
    // Initialize pixel buffer
    pixel_buffer_.resize(width_ * height_ * 3);  // RGB
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

void Window::update() {
    // For now, just swap buffers - actual drawing happens in display_framebuffer
    glfwSwapBuffers(window_);
}

void Window::swap_buffers() {
    glfwSwapBuffers(window_);
}

void Window::display_framebuffer(const rendering::Framebuffer& framebuffer) {
    // Convert framebuffer to pixel buffer for potential future display
    int w = framebuffer.width();
    int h = framebuffer.height();
    
    // Convert float colors to byte values
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            auto color = framebuffer.get_pixel(x, y);
            int idx = (y * w + x) * 3;
            
            // Apply gamma correction and clamp
            float r = std::min(1.0f, std::max(0.0f, std::pow(color.r, 1.0f/2.2f)));
            float g = std::min(1.0f, std::max(0.0f, std::pow(color.g, 1.0f/2.2f)));
            float b = std::min(1.0f, std::max(0.0f, std::pow(color.b, 1.0f/2.2f)));
            
            pixel_buffer_[idx + 0] = static_cast<unsigned char>(r * 255);
            pixel_buffer_[idx + 1] = static_cast<unsigned char>(g * 255);
            pixel_buffer_[idx + 2] = static_cast<unsigned char>(b * 255);
        }
    }
    
    // Display framebuffer info and sample colors
    std::cout << "=== FRAMEBUFFER DISPLAY ===" << std::endl;
    std::cout << "Size: " << w << "x" << h << std::endl;
    std::cout << "Data pointer: " << framebuffer.data() << std::endl;
    
    // Show sample pixels from different areas
    std::cout << "\nSample pixels:" << std::endl;
    auto tl = framebuffer.get_pixel(0, h-1);
    std::cout << "Top-left: (" << tl.r << ", " << tl.g << ", " << tl.b << ")" << std::endl;
    
    auto center = framebuffer.get_pixel(w/2, h/2);
    std::cout << "Center: (" << center.r << ", " << center.g << ", " << center.b << ")" << std::endl;
    
    auto tr = framebuffer.get_pixel(w-1, h-1);
    std::cout << "Top-right: (" << tr.r << ", " << tr.g << ", " << tr.b << ")" << std::endl;
    
    auto bl = framebuffer.get_pixel(0, 0);
    std::cout << "Bottom-left: (" << bl.r << ", " << bl.g << ", " << bl.b << ")" << std::endl;
    
    auto br = framebuffer.get_pixel(w-1, 0);
    std::cout << "Bottom-right: (" << br.r << ", " << br.g << ", " << br.b << ")" << std::endl;
    
    std::cout << "\nWindow is open - close it manually to continue." << std::endl;
    std::cout << "Image saved to: cornell_box.ppm" << std::endl;
}

void Window::draw_pixels() {
    // Placeholder for pixel drawing - could be implemented with OpenGL or other methods
    // For now, this is handled by the console output and file saving
}

} // namespace viewer
} // namespace raytracer
