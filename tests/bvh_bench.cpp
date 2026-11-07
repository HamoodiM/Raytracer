/**
 * @file bvh_bench.cpp
 * @brief Brute force vs BVH ray casting on meshes of increasing size
 *
 * Each scene is a bumpy sphere tessellated into 2 * n * n triangles. A
 * 128 x 128 grid of camera rays is cast at it (closest hit, like primary
 * rays). Brute force uses a subset of the rays when the mesh is large, so
 * every measurement runs for a reasonable time; rates are reported as rays
 * per second, and the brute-force subset is checked against the BVH result.
 *
 *   bvh_bench [max_n]      (default 512 -> up to 524,288 triangles)
 */

#include "core/scene.h"
#include "geometry/triangle.h"
#include "materials/lambertian.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <vector>

using namespace raytracer;
using Clock = std::chrono::steady_clock;

namespace {

double seconds_since(Clock::time_point t0) {
    return std::chrono::duration<double>(Clock::now() - t0).count();
}

Point3 bumpy_sphere(float theta, float phi) {
    float r = 1.0f + 0.08f * std::sin(7.0f * theta) * std::sin(9.0f * phi);
    return r * Point3(std::sin(theta) * std::cos(phi), std::cos(theta), std::sin(theta) * std::sin(phi));
}

void add_mesh(core::Scene& scene, int n) {
    auto material = std::make_shared<materials::Lambertian>(Color(0.7f));
    const float pi = 3.14159265f;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            float t0 = pi * i / n, t1 = pi * (i + 1) / n;
            float p0 = 2 * pi * j / n, p1 = 2 * pi * (j + 1) / n;
            Point3 a = bumpy_sphere(t0, p0), b = bumpy_sphere(t1, p0);
            Point3 c = bumpy_sphere(t1, p1), d = bumpy_sphere(t0, p1);
            scene.add(std::make_shared<geometry::Triangle>(a, b, c, material));
            scene.add(std::make_shared<geometry::Triangle>(a, c, d, material));
        }
    }
}

std::vector<core::Ray> camera_rays(int res) {
    std::vector<core::Ray> rays;
    Point3 eye(0.4f, 0.6f, 3.2f);
    Vec3 forward = glm::normalize(Point3(0.0f) - eye);
    Vec3 right = glm::normalize(glm::cross(forward, Vec3(0, 1, 0)));
    Vec3 up = glm::cross(right, forward);
    for (int y = 0; y < res; ++y)
        for (int x = 0; x < res; ++x) {
            float u = ((x + 0.5f) / res - 0.5f) * 0.9f, v = ((y + 0.5f) / res - 0.5f) * 0.9f;
            rays.emplace_back(eye, glm::normalize(forward + u * right + v * up));
        }
    return rays;
}

struct Pass {
    double seconds;
    int hits;
    std::vector<float> t;
};

Pass cast(const core::Scene& scene, const std::vector<core::Ray>& rays, size_t count, bool brute) {
    Pass p{0.0, 0, std::vector<float>(count, -1.0f)};
    auto t0 = Clock::now();
    for (size_t i = 0; i < count; ++i) {
        geometry::HitRecord rec;
        bool h = brute ? scene.hit_brute_force(rays[i], 0.001f, INFINITY, rec)
                       : scene.hit(rays[i], 0.001f, INFINITY, rec);
        if (h) {
            p.hits++;
            p.t[i] = rec.t;
        }
    }
    p.seconds = seconds_since(t0);
    return p;
}

} // namespace

int main(int argc, char** argv) {
    const int max_n = argc > 1 ? std::atoi(argv[1]) : 512;
    const std::vector<core::Ray> rays = camera_rays(128);
    const double brute_budget = 4e8;  // triangle tests per brute-force measurement

    std::printf("| triangles | SAH build (ms) | BVH nodes | BVH SAH (Mrays/s) | BVH midpoint (Mrays/s) | "
                "brute force (Mrays/s) | brute rays | speed-up SAH vs brute |\n");
    std::printf("|---:|---:|---:|---:|---:|---:|---:|---:|\n");

    int mismatches = 0;
    for (int n : {8, 23, 64, 128, 256, 512}) {
        if (n > max_n) break;
        core::Scene scene;
        add_mesh(scene, n);
        const size_t tris = scene.object_count();

        auto tb = Clock::now();
        scene.build_acceleration(acceleration::BVH::SplitMethod::SAH);
        const double build_ms = seconds_since(tb) * 1000.0;
        const size_t nodes = scene.bvh().stats().nodes;

        Pass sah = cast(scene, rays, rays.size(), false);
        for (int rep = 0; rep < 2; ++rep) sah.seconds = std::min(sah.seconds, cast(scene, rays, rays.size(), false).seconds);

        // Brute force on a subset of the same rays, spread over the image
        size_t brute_count = std::min(rays.size(), static_cast<size_t>(brute_budget / tris));
        brute_count = std::max<size_t>(brute_count, 64);
        std::vector<core::Ray> subset;
        size_t stride = rays.size() / brute_count;
        for (size_t i = 0; i < rays.size() && subset.size() < brute_count; i += stride) subset.push_back(rays[i]);
        Pass brute = cast(scene, subset, subset.size(), true);
        for (size_t k = 0; k < subset.size(); ++k) {
            float bvh_t = sah.t[k * stride];
            if (std::fabs(bvh_t - brute.t[k]) > 1e-5f * std::max(1.0f, std::fabs(brute.t[k]))) mismatches++;
        }

        scene.build_acceleration(acceleration::BVH::SplitMethod::Middle);
        Pass mid = cast(scene, rays, rays.size(), false);
        for (int rep = 0; rep < 2; ++rep) mid.seconds = std::min(mid.seconds, cast(scene, rays, rays.size(), false).seconds);

        double sah_rate = rays.size() / sah.seconds, mid_rate = rays.size() / mid.seconds;
        double brute_rate = subset.size() / brute.seconds;
        std::printf("| %zu | %.1f | %zu | %.3f | %.3f | %.5f | %zu | %.0fx |\n", tris, build_ms, nodes,
                    sah_rate / 1e6, mid_rate / 1e6, brute_rate / 1e6, subset.size(), sah_rate / brute_rate);
        std::fflush(stdout);
    }
    std::printf("\n%d mismatches between BVH and brute-force hit distances\n", mismatches);
    return mismatches ? 1 : 0;
}
