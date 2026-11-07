#include "utils.h"
#include <random>

namespace raytracer {
namespace core {

namespace {

// PCG32 (O'Neill, pcg-random.org): 64-bit state, 32-bit output. Small and
// cheap to reseed, which matters because renderers reseed once per pixel.
struct Pcg32 {
    uint64_t state = 0x853c49e6748fea9bULL;
    uint64_t inc = 0xda3e39cb94b95bdbULL;

    void seed(uint64_t init_state, uint64_t sequence) {
        state = 0;
        inc = (sequence << 1u) | 1u;
        next();
        state += init_state;
        next();
    }
    uint32_t next() {
        uint64_t old = state;
        state = old * 6364136223846793005ULL + inc;
        uint32_t xorshifted = static_cast<uint32_t>(((old >> 18u) ^ old) >> 27u);
        uint32_t rot = static_cast<uint32_t>(old >> 59u);
        return (xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31u));
    }
};

// One engine per thread. Only the engine is persistent: the [min, max)
// range is applied on every call (a static distribution object would keep
// the range of the first call).
thread_local Pcg32 tls_rng;

uint64_t splitmix64(uint64_t x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

} // namespace

void seed_rng(uint64_t seed) {
    tls_rng.seed(splitmix64(seed), splitmix64(seed ^ 0x5851f42d4c957f2dULL));
}

void seed_rng_for_pixel(int x, int y, uint32_t pass, uint64_t base_seed) {
    uint64_t key = (static_cast<uint64_t>(static_cast<uint32_t>(y)) << 32) | static_cast<uint32_t>(x);
    seed_rng(splitmix64(key) ^ splitmix64(base_seed + 0x632be59bd9b4e019ULL * (pass + 1ull)));
}

// Uniform float in [min, max). The range is applied on every call.
float random_float(float min, float max) {
    // 24 random bits -> exactly representable float in [0, 1)
    float unit = static_cast<float>(tls_rng.next() >> 8) * (1.0f / 16777216.0f);
    return min + (max - min) * unit;
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
    // Inclusive range [min, max]
    int span = max - min + 1;
    int value = min + static_cast<int>(random_float() * static_cast<float>(span));
    return value > max ? max : value;
}

} // namespace core
} // namespace raytracer
