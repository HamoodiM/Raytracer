/**
 * @file smooth_normals_test.cpp
 * @brief Tests for smooth-shaded triangles and vertex-normal generation
 *
 * 1. compute_vertex_normals: an octahedron's vertex normals are the axis
 *    directions, and faces are weighted by area.
 * 2. Triangle with vertex normals: on a tessellated unit sphere whose
 *    vertex normals are the vertex positions, the interpolated shading
 *    normal must equal normalize(hit point) (this checks the barycentric
 *    weights), while flat face normals are off by several degrees; the
 *    geometric normal stays the face normal; hits from inside flip both.
 * 3. OBJ loading: "smooth" uses the file's vn normals (re-oriented to the
 *    winding if they point the other way), computes them when vn is missing,
 *    and "smooth": false keeps flat shading.
 *
 * Writes a temporary OBJ file in the current directory. Exit code 0 = pass.
 */

#include "core/scene.h"
#include "core/scene_loader.h"
#include "geometry/mesh_normals.h"
#include "geometry/triangle.h"
#include "materials/lambertian.h"
#include <cmath>
#include <cstdio>
#include <memory>
#include <random>

using namespace raytracer;

namespace {

int failures = 0;

void check(bool ok, const char* what) {
    std::printf("%s  %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) failures++;
}

bool near(const Vec3& a, const Vec3& b, float eps = 1e-4f) { return glm::length(a - b) < eps; }

float angle_deg(const Vec3& a, const Vec3& b) {
    float c = glm::clamp(glm::dot(glm::normalize(a), glm::normalize(b)), -1.0f, 1.0f);
    return std::acos(c) * 57.2957795f;
}

void test_vertex_normals() {
    // Octahedron, faces wound counter-clockwise seen from outside
    std::vector<Point3> v = {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};
    std::vector<std::array<int, 3>> f = {{0, 2, 4}, {2, 1, 4}, {1, 3, 4}, {3, 0, 4},
                                         {2, 0, 5}, {1, 2, 5}, {3, 1, 5}, {0, 3, 5}};
    auto n = geometry::compute_vertex_normals(v, f);
    bool ok = true;
    for (size_t i = 0; i < v.size(); ++i) ok = ok && near(n[i], v[i]);
    check(ok, "octahedron vertex normals equal the axis directions (outward)");

    // Area weighting: vertex 0 is shared by a face of area 2 facing +z and a
    // face of area 0.5 facing +x, so its normal is normalize(2 z + 0.5 x).
    std::vector<Point3> w = {{0, 0, 0}, {2, 0, 0}, {0, 2, 0}, {0, 0, 1}, {0, 1, 0}};
    std::vector<std::array<int, 3>> g = {{0, 1, 2}, {0, 4, 3}};
    auto m = geometry::compute_vertex_normals(w, g);
    check(near(m[0], glm::normalize(Vec3(0.5f, 0.0f, 2.0f))), "vertex normals are area weighted");
}

void test_sphere() {
    auto mat = std::make_shared<materials::Lambertian>(Color(0.5f));
    core::Scene smooth, flat;
    const int n = 16;
    const float pi = 3.14159265f;
    auto P = [&](int i, int j) {
        float t = pi * i / n, p = 2 * pi * j / n;
        return Point3(std::sin(t) * std::cos(p), std::cos(t), std::sin(t) * std::sin(p));
    };
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j) {
            Point3 a = P(i, j), b = P(i + 1, j), c = P(i + 1, j + 1), d = P(i, j + 1);
            // Wound so cross(e1, e2) points outward. Vertex normals are the
            // exact sphere normals (the positions on a unit sphere).
            smooth.add(std::make_shared<geometry::Triangle>(a, c, b, a, c, b, mat));
            smooth.add(std::make_shared<geometry::Triangle>(a, d, c, a, d, c, mat));
            flat.add(std::make_shared<geometry::Triangle>(a, c, b, mat));
            flat.add(std::make_shared<geometry::Triangle>(a, d, c, mat));
        }
    smooth.build_acceleration();
    flat.build_acceleration();

    std::mt19937 rng(7);
    std::uniform_real_distribution<float> U(-1.0f, 1.0f);
    float max_smooth = 0, max_flat = 0, sum_smooth = 0, sum_flat = 0;
    bool geometric_ok = true, faces_ray = true, inside_ok = true;
    int hits = 0;
    for (int k = 0; k < 5000; ++k) {
        Vec3 target(U(rng), U(rng), U(rng));
        if (glm::length(target) < 1e-3f) continue;
        target = glm::normalize(target);
        Point3 origin = target * 3.0f + Vec3(U(rng), U(rng), U(rng)) * 0.3f;
        core::Ray ray(origin, target * 0.9f - origin);
        geometry::HitRecord s, f;
        if (!smooth.hit(ray, 0.001f, INFINITY, s) || !flat.hit(ray, 0.001f, INFINITY, f)) continue;
        hits++;
        Vec3 truth = glm::normalize(s.point);
        float es = angle_deg(s.normal, truth), ef = angle_deg(f.normal, truth);
        max_smooth = std::fmax(max_smooth, es);
        max_flat = std::fmax(max_flat, ef);
        sum_smooth += es;
        sum_flat += ef;
        geometric_ok = geometric_ok && near(s.geometric_normal, f.normal, 1e-4f) && s.front_face;
        faces_ray = faces_ray && glm::dot(s.normal, ray.direction()) < 0.0f;

        // Same point hit from inside the sphere: both normals face inward
        core::Ray inner(Point3(0.0f), s.point);
        geometry::HitRecord in;
        if (smooth.hit(inner, 0.001f, INFINITY, in)) {
            inside_ok = inside_ok && !in.front_face && glm::dot(in.normal, in.point) < 0.0f &&
                        glm::dot(in.geometric_normal, in.point) < 0.0f;
        }
    }
    std::printf("      %d hits; angle to true normal: smooth mean %.3f max %.3f deg, flat mean %.3f max %.3f deg\n",
                hits, sum_smooth / hits, max_smooth, sum_flat / hits, max_flat);
    check(hits > 4000, "rays hit the tessellated sphere");
    // With vertex normal = vertex position, the barycentric interpolation of
    // the normals equals the interpolated hit point, so normalize(point) is
    // the exact expected value: any mix-up of the barycentric weights shows.
    check(max_smooth < 0.1f, "interpolated normal matches the barycentric expectation (< 0.1 deg)");
    check(max_flat > 5.0f, "flat face normals deviate by several degrees on this mesh");
    check(geometric_ok, "geometric_normal is the face normal and front_face is set from it");
    check(faces_ray, "shading normal faces the incoming ray");
    check(inside_ok, "hits from inside flip both normals");
}

bool load_quad_normal(const char* obj_text, bool smooth, Vec3& normal, Vec3& geometric) {
    const char* path = "smooth_normals_test_tmp.obj";
    std::FILE* f = std::fopen(path, "wb");
    if (!f) return false;
    std::fputs(obj_text, f);
    std::fclose(f);
    nlohmann::json j = {{"objects",
                         {{{"type", "obj"}, {"file", path}, {"smooth", smooth},
                           {"material", {{"type", "lambertian"}, {"albedo", {0.5, 0.5, 0.5}}}}}}}};
    core::Scene scene = core::SceneLoader::load_scene(j);
    std::remove(path);
    geometry::HitRecord rec;
    // Quad in the z = 0 plane, wound counter-clockwise seen from +z; hit it from +z
    if (!scene.hit(core::Ray(Point3(0.3f, 0.2f, 5.0f), Vec3(0, 0, -1)), 0.001f, INFINITY, rec)) return false;
    normal = rec.normal;
    geometric = rec.geometric_normal;
    return true;
}

void test_obj() {
    const char* quad_vn =
        "v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\n"
        "vn 0 1 1\n"
        "f 1//1 2//1 3//1 4//1\n";
    const char* quad_vn_flipped =
        "v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\n"
        "vn 0 -1 -1\n"
        "f 1/1/1 2/1/1 3/1/1 4/1/1\n";
    const char* quad_plain = "v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\nf 1 2 3 4\n";
    const Vec3 tilted = glm::normalize(Vec3(0, 1, 1));
    Vec3 n, g;
    check(load_quad_normal(quad_vn, true, n, g) && near(n, tilted) && near(g, Vec3(0, 0, 1)),
          "OBJ smooth: uses vn normals, geometric normal unchanged");
    check(load_quad_normal(quad_vn_flipped, true, n, g) && near(n, tilted),
          "OBJ smooth: vn pointing against the winding is re-oriented");
    check(load_quad_normal(quad_vn, false, n, g) && near(n, Vec3(0, 0, 1)),
          "OBJ \"smooth\": false keeps flat shading");
    check(load_quad_normal(quad_plain, true, n, g) && near(n, Vec3(0, 0, 1)),
          "OBJ smooth without vn: computed normals (flat quad -> face normal)");
}

} // namespace

int main() {
    test_vertex_normals();
    test_sphere();
    test_obj();
    std::printf("%s: %d failure(s)\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
