#pragma once

#include "../common.h"
#include <cstdint>
#include <random>

namespace raytracer {
namespace core {

// Random number generation.
// Each thread has its own engine (thread_local), so these are safe to call
// from OpenMP worker threads. Renderers reseed per pixel and sample pass with
// seed_rng_for_pixel(), which makes images independent of thread scheduling.
void seed_rng(uint64_t seed);
void seed_rng_for_pixel(int x, int y, uint32_t pass, uint64_t base_seed = 0);
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
