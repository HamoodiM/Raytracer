#include "utils.h"
#include <random>

namespace raytracer {
namespace core {

// Helper function for random float
float random_float(float min, float max) {
    static std::uniform_real_distribution<float> distribution(min, max);
    static std::mt19937 generator;
    return distribution(generator);
}

// Helper function for random point in unit disk
Vec3 random_in_unit_disk() {
    while (true) {
        auto p = Vec3(random_float(-1, 1), random_float(-1, 1), 0);
        if (glm::dot(p, p) >= 1) continue;
        return p;
    }
}

// Helper function for random point in unit sphere
Vec3 random_in_unit_sphere() {
    while (true) {
        auto p = Vec3(random_float(-1, 1), random_float(-1, 1), random_float(-1, 1));
        if (glm::dot(p, p) >= 1) continue;
        return p;
    }
}

// Helper function for random unit vector
Vec3 random_unit_vector() {
    return glm::normalize(random_in_unit_sphere());
}

// Utility functions
float clamp(float x, float min, float max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

Vec3 reflect(const Vec3& v, const Vec3& n) {
    return v - 2.0f * glm::dot(v, n) * n;
}

Color gamma_correct(const Color& c, float gamma) {
    return Color(pow(c.r, 1.0f/gamma), pow(c.g, 1.0f/gamma), pow(c.b, 1.0f/gamma));
}

// Helper function for random integer
int random_int(int min, int max) {
    static std::uniform_int_distribution<int> distribution(min, max);
    static std::mt19937 generator;
    return distribution(generator);
}

} // namespace core
} // namespace raytracer
