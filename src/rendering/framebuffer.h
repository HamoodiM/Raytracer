#pragma once

#include "../common.h"
#include "../core/ray.h"
#include <cstdint>
#include <string>
#include <vector>

namespace raytracer {
namespace rendering {

class Framebuffer {
public:
    Framebuffer(int width, int height);
    
    void set_pixel(int x, int y, const Color& color);
    Color get_pixel(int x, int y) const;
    
    /**
     * @brief Adds a sample to the accumulation buffer for progressive rendering
     * 
     * @param x Pixel x coordinate
     * @param y Pixel y coordinate
     * @param color Sample color to accumulate
     */
    void add_sample(int x, int y, const Color& color);
    
    /**
     * @brief Updates the display buffer from the accumulation buffer
     * 
     * @param sample_count Number of samples accumulated so far
     */
    void update_display(int sample_count);
    
    int width() const { return width_; }
    int height() const { return height_; }
    
    void clear(const Color& color = Color(0, 0, 0));
    void save_ppm(const std::string& filename) const;  // binary PPM (P6)
    /// Writes PNG or PPM depending on the extension; returns false on error
    bool save(const std::string& filename) const;
    /// Display buffer as 8-bit sRGB-ish (gamma 2.2) RGB, rows top to bottom
    std::vector<uint8_t> to_rgb8() const;
    
    const Color* data() const { return pixels_.data(); }

private:
    int width_, height_;
    std::vector<Color> pixels_;           // Display buffer
    std::vector<Color> accumulation_;     // Accumulation buffer for progressive rendering
};

} // namespace rendering
} // namespace raytracer
