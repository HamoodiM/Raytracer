#include "window.h"
#include <cstring>
#include <iostream>

namespace raytracer {
namespace viewer {

Window::Window(int width, int height, const std::string& title)
    : width_(width), height_(height) {

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    // Compatibility context: no minimum version and no core profile, so the
    // driver returns its compatibility context (legacy 2.1 on macOS, 1.1 on
    // Windows' software fallback). The OpenGL 1.1 pixel calls used below
    // exist in all of them and are exported by the system GL library, so no
    // function loader is needed.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window_ = glfwCreateWindow(width_, height_, title.c_str(), nullptr, nullptr);
    if (!window_) {
        std::cerr << "Failed to create GLFW window (OpenGL compatibility context)" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);

    pixel_buffer_.resize(static_cast<size_t>(width_) * height_ * 3);
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
    const int w = framebuffer.width();
    const int h = framebuffer.height();

    // to_rgb8() is top-down; glDrawPixels fills rows bottom-up.
    const std::vector<uint8_t> rgb = framebuffer.to_rgb8();
    const size_t row_bytes = static_cast<size_t>(w) * 3;
    pixel_buffer_.resize(row_bytes * h);
    for (int y = 0; y < h; ++y) {
        std::memcpy(&pixel_buffer_[(h - 1 - y) * row_bytes], &rgb[y * row_bytes], row_bytes);
    }

    // The drawable can be larger than the window on high-DPI screens; scale to fill it.
    int fb_w = 0, fb_h = 0;
    glfwGetFramebufferSize(window_, &fb_w, &fb_h);
    glViewport(0, 0, fb_w, fb_h);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelZoom(static_cast<float>(fb_w) / w, static_cast<float>(fb_h) / h);
    glRasterPos2f(-1.0f, -1.0f);  // bottom-left corner of the viewport
    glDrawPixels(w, h, GL_RGB, GL_UNSIGNED_BYTE, pixel_buffer_.data());
}

std::vector<uint8_t> Window::read_back_rgb8(int& width, int& height) const {
    glfwGetFramebufferSize(window_, &width, &height);
    const size_t row_bytes = static_cast<size_t>(width) * 3;
    std::vector<uint8_t> bottom_up(row_bytes * height);

    glFinish();
    glReadBuffer(GL_BACK);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, bottom_up.data());

    std::vector<uint8_t> top_down(bottom_up.size());
    for (int y = 0; y < height; ++y) {
        std::memcpy(&top_down[y * row_bytes], &bottom_up[(height - 1 - y) * row_bytes], row_bytes);
    }
    return top_down;
}

} // namespace viewer
} // namespace raytracer
