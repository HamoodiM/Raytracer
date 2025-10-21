#pragma once

#include "../common.h"
#include "../core/ray.h"
#include <vector>

namespace raytracer {
namespace rendering {

class Framebuffer {
public:
    Framebuffer(int width, int height);
    
    void set_pixel(int x, int y, const Color& color);
    Color get_pixel(int x, int y) const;
    
    int width() const { return width_; }
    int height() const { return height_; }
    
    void clear(const Color& color = Color(0, 0, 0));
    void save_ppm(const std::string& filename) const;
    
    const Color* data() const { return pixels_.data(); }

private:
    int width_, height_;
    std::vector<Color> pixels_;
};

} // namespace rendering
} // namespace raytracer
