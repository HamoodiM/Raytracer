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

} // namespace core
} // namespace raytracer
