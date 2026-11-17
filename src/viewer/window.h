#pragma once

#include <GLFW/glfw3.h>
#include "../rendering/framebuffer.h"
#include <cstdint>
#include <string>
#include <vector>

namespace raytracer {
namespace viewer {

/**
 * GLFW window that shows the progressive image.
 *
 * Drawing uses only OpenGL 1.1 calls (glDrawPixels, glReadPixels), which the
 * system GL library exports directly, so no function loader is needed. The
 * window asks for a compatibility context (no core profile), where these
 * calls are always available.
 */
class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool should_close() const;
    void poll_events();
    void swap_buffers();

    /// Draws the framebuffer's current display image into the back buffer
    /// (gamma 2.2, 8-bit, the same conversion as the saved images).
    void display_framebuffer(const rendering::Framebuffer& framebuffer);

    /// Reads the back buffer as 8-bit RGB, rows top to bottom (the layout
    /// Framebuffer::to_rgb8 uses). Call before swap_buffers.
    std::vector<uint8_t> read_back_rgb8(int& width, int& height) const;

    GLFWwindow* get_handle() const { return window_; }
    int width() const { return width_; }
    int height() const { return height_; }

private:
    GLFWwindow* window_ = nullptr;
    int width_, height_;
    std::vector<uint8_t> pixel_buffer_;  // RGB, rows bottom to top (GL order)
};

} // namespace viewer
} // namespace raytracer
