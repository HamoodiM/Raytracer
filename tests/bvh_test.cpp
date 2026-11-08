/**
 * @file bvh_test.cpp
 * @brief Checks that BVH traversal returns exactly the brute-force closest hit
 *
 * Builds random scenes (triangles of mixed sizes, spheres, one infinite
 * plane), fires random rays with both infinite and finite t_max (the latter
 * like shadow rays), and compares Scene::hit (BVH) against
 * Scene::hit_brute_force for every ray: same hit/miss, same t, and the same
 * primitive (each primitive has its own material object, used as an ID).
 *
 * Exit code 0 = all rays agree, 1 = at least one mismatch.
 */

#include "core/scene.h"
#include "geometry/plane.h"
#include "geometry/sphere.h"
#include "geometry/triangle.h"
#include "materials/lambertian.h"
#include <cmath>
#include <cstdio>
#include <memory>
#include <random>

using namespace raytracer;

namespace {

struct Rng {
    std::mt19937 gen;
    explicit Rng(unsigned seed) : gen(seed) {}
    float uniform(float a, float b) { return std::uniform_real_distribution<float>(a, b)(gen); }
    Vec3 in_box(float h) { return Vec3(uniform(-h, h), uniform(-h, h), uniform(-h, h)); }
    Vec3 direction() {
        while (true) {
            Vec3 d = in_box(1.0f);
            float l2 = glm::dot(d, d);
            if (l2 > 1e-4f && l2 <= 1.0f) return d / std::sqrt(l2);
        }
    }
};

std::shared_ptr<materials::Material> unique_material() {
    return std::make_shared<materials::Lambertian>(Color(0.5f));
}

core::Scene random_scene(Rng& rng, int triangles, int spheres, bool with_plane) {
    core::Scene scene;
    for (int i = 0; i < triangles; ++i) {
        Point3 c = rng.in_box(10.0f);
        float s = (i % 10 == 0) ? 3.0f : 0.4f;  // mostly small, some large
        scene.add(std::make_shared<geometry::Triangle>(c + rng.in_box(s), c + rng.in_box(s),
                                                      c + rng.in_box(s), unique_material()));
    }
    for (int i = 0; i < spheres; ++i) {
        scene.add(std::make_shared<geometry::Sphere>(rng.in_box(10.0f), rng.uniform(0.05f, 1.0f),
                                                    unique_material()));
    }
    // A few axis-aligned triangles (zero thickness on one axis; Triangle::bounding_box pads them by 1e-4)
    for (int i = 0; i < 20; ++i) {
        Point3 c = rng.in_box(8.0f);
        scene.add(std::make_shared<geometry::Triangle>(c, c + Vec3(1, 0, 0), c + Vec3(0, 0, 1),
                                                      unique_material()));
    }
    if (with_plane) {
        scene.add(std::make_shared<geometry::Plane>(Point3(0, -9, 0), Vec3(0, 1, 0), unique_material()));
    }
    return scene;
}

} // namespace

int main() {
    const struct { int triangles, spheres; bool plane; } configs[] = {
        {1, 0, false}, {10, 3, false}, {500, 50, true}, {5000, 200, true}, {20000, 0, false},
    };
    const char* method_name[] = {"SAH", "Middle"};
    const acceleration::BVH::SplitMethod methods[] = {acceleration::BVH::SplitMethod::SAH,
                                                      acceleration::BVH::SplitMethod::Middle};
    const int rays_per_scene = 20000;

    Rng rng(12345);
    long long total_rays = 0, total_hits = 0, failures = 0;

    for (const auto& cfg : configs) {
        core::Scene scene = random_scene(rng, cfg.triangles, cfg.spheres, cfg.plane);
        for (int m = 0; m < 2; ++m) {
            scene.build_acceleration(methods[m]);
            const auto& st = scene.bvh().stats();
            int hits = 0, scene_failures = 0;
            for (int i = 0; i < rays_per_scene; ++i) {
                // Half the rays start outside looking in, half start inside the scene
                Point3 origin = (i % 2) ? rng.in_box(15.0f) : rng.in_box(5.0f);
                core::Ray ray(origin, rng.direction() * rng.uniform(0.5f, 2.0f));  // unnormalized too
                float t_max = (i % 3 == 0) ? rng.uniform(0.5f, 20.0f) : INFINITY;

                geometry::HitRecord a, b;
                bool ha = scene.hit(ray, 0.001f, t_max, a);
                bool hb = scene.hit_brute_force(ray, 0.001f, t_max, b);
                bool ok = (ha == hb);
                if (ok && ha) {
                    ok = std::fabs(a.t - b.t) <= 1e-5f * std::fmax(1.0f, b.t) &&
                         (a.material == b.material || a.t == b.t);  // exact tie: either is valid
                }
                if (!ok) {
                    if (scene_failures < 5)
                        std::printf("  MISMATCH ray %d: bvh=%d t=%g  brute=%d t=%g\n", i, ha,
                                    ha ? a.t : 0.0, hb, hb ? b.t : 0.0);
                    scene_failures++;
                }
                hits += hb;
            }
            std::printf("%-6s tris=%-6d spheres=%-4d plane=%d | nodes=%-6zu leaves=%-6zu depth=%-3zu "
                        "max_leaf=%zu | rays=%d hits=%d mismatches=%d\n",
                        method_name[m], cfg.triangles + 20, cfg.spheres, cfg.plane ? 1 : 0, st.nodes,
                        st.leaves, st.max_depth, st.max_leaf_size, rays_per_scene, hits,
                        scene_failures);
            total_rays += rays_per_scene;
            total_hits += hits;
            failures += scene_failures;
        }
    }

    std::printf("%s: %lld rays, %lld hits, %lld mismatches between BVH and brute force\n",
                failures ? "FAIL" : "PASS", total_rays, total_hits, failures);
    return failures ? 1 : 0;
}
