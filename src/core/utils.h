#pragma once

#include "../common.h"
#include <random>

namespace raytracer {
namespace core {

// Random number generation
float random_float(float min = 0.0f, float max = 1.0f);
int random_int(int min, int max);
Vec3 random_in_unit_disk();
Vec3 random_in_unit_sphere();
Vec3 random_unit_vector();

// Utility functions
float clamp(float x, float min, float max);
Vec3 reflect(const Vec3& v, const Vec3& n);
Color gamma_correct(const Color& c, float gamma = 2.2f);

} // namespace core
} // namespace raytracer
