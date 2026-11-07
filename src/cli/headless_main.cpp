/**
 * @file headless_main.cpp
 * @brief Command-line renderer (no window, no GLFW/OpenGL)
 *
 *   raytracer_cli --scene scenes/cornell_box.json --spp 64 --out image.png
 *                 [--width 640] [--height 360] [--depth 50] [--seed 0]
 *                 [--rr 3] [--no-bvh] [--bvh-split sah|middle] [--threads N]
 *                 [--snapshots pow2|all --snapshot-prefix renders/frame]
 *
 * The camera comes from the scene file; its aspect ratio is replaced by
 * width / height. Output format follows the extension (.png or .ppm).
 */

#include "../core/scene_loader.h"
#include "../core/utils.h"
#include "../rendering/framebuffer.h"
#include "../rendering/integrator.h"
#include "../rendering/renderer.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

#if defined(_OPENMP)
#include <omp.h>
#endif

using namespace raytracer;

namespace {

struct Options {
    std::string scene;
    std::string out = "out.png";
    int width = 640;
    int height = 360;
    int spp = 64;
    int depth = 50;
    uint64_t seed = 0;
    int rr_start = 3;
    bool use_bvh = true;
    std::string bvh_split = "sah";
    int threads = 0;
    std::string snapshots;  // "", "pow2" or "all"
    std::string snapshot_prefix = "frame";
};

void print_usage() {
    std::cout <<
        "Usage: raytracer_cli --scene FILE.json [options]\n"
        "  --out FILE           output image, .png or .ppm (default out.png)\n"
        "  --width N            image width (default 640)\n"
        "  --height N           image height (default 360)\n"
        "  --spp N              samples per pixel (default 64)\n"
        "  --depth N            max path length (default 50)\n"
        "  --seed N             RNG seed (default 0)\n"
        "  --rr N               Russian roulette after N bounces, 0 = off (default 3)\n"
        "  --no-bvh             brute-force intersection (for comparison)\n"
        "  --bvh-split sah|middle   BVH split method (default sah)\n"
        "  --threads N          OpenMP threads (ignored without OpenMP)\n"
        "  --snapshots pow2|all also write the accumulated image after every\n"
        "                       power-of-two / every sample count\n"
        "  --snapshot-prefix P  snapshot files are P_sppNNNN.<ext of --out>\n";
}

bool parse_args(int argc, char** argv, Options& o) {
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto next = [&]() -> std::string {
            if (i + 1 >= argc) throw std::runtime_error("missing value for " + a);
            return argv[++i];
        };
        if (a == "--scene") o.scene = next();
        else if (a == "--out") o.out = next();
        else if (a == "--width") o.width = std::stoi(next());
        else if (a == "--height") o.height = std::stoi(next());
        else if (a == "--spp") o.spp = std::stoi(next());
        else if (a == "--depth") o.depth = std::stoi(next());
        else if (a == "--seed") o.seed = std::stoull(next());
        else if (a == "--rr") o.rr_start = std::stoi(next());
        else if (a == "--no-bvh") o.use_bvh = false;
        else if (a == "--bvh-split") o.bvh_split = next();
        else if (a == "--threads") o.threads = std::stoi(next());
        else if (a == "--snapshots") o.snapshots = next();
        else if (a == "--snapshot-prefix") o.snapshot_prefix = next();
        else if (a == "--help" || a == "-h") return false;
        else throw std::runtime_error("unknown option " + a);
    }
    if (o.scene.empty()) throw std::runtime_error("--scene is required");
    if (o.width < 2 || o.height < 2 || o.spp < 1 || o.depth < 1)
        throw std::runtime_error("width/height must be >= 2, spp and depth >= 1");
    if (!o.snapshots.empty() && o.snapshots != "pow2" && o.snapshots != "all")
        throw std::runtime_error("--snapshots must be pow2 or all");
    if (o.bvh_split != "sah" && o.bvh_split != "middle")
        throw std::runtime_error("--bvh-split must be sah or middle");
    return true;
}

std::string extension_of(const std::string& path) {
    size_t dot = path.find_last_of('.');
    return dot == std::string::npos ? ".ppm" : path.substr(dot);
}

// Progressive accumulation, one sample per pixel per pass, so intermediate
// images can be written. Same estimator as Renderer::render.
void render_progressive(const Options& o, const core::Camera& camera, const core::Scene& scene,
                        const rendering::Integrator& integrator, rendering::Framebuffer& fb) {
    const int w = fb.width(), h = fb.height();
    const std::string ext = extension_of(o.out);
    for (int pass = 0; pass < o.spp; ++pass) {
#if defined(_OPENMP)
#pragma omp parallel for schedule(dynamic, 1)
#endif
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                core::seed_rng_for_pixel(x, y, static_cast<uint32_t>(pass), o.seed);
                float u = (x + core::random_float()) / (w - 1);
                float v = (y + core::random_float()) / (h - 1);
                fb.add_sample(x, y, integrator.trace(camera.get_ray(u, v), scene, o.depth));
            }
        }
        const int spp_done = pass + 1;
        const bool pow2 = (spp_done & (spp_done - 1)) == 0;
        if (o.snapshots == "all" || (o.snapshots == "pow2" && pow2)) {
            fb.update_display(spp_done);
            char name[32];
            std::snprintf(name, sizeof(name), "_spp%04d", spp_done);
            std::string file = o.snapshot_prefix + name + ext;
            if (!fb.save(file)) throw std::runtime_error("could not write " + file);
            std::cout << "snapshot " << file << std::endl;
        }
    }
    fb.update_display(o.spp);
}

} // namespace

int main(int argc, char** argv) {
    try {
        Options o;
        if (!parse_args(argc, argv, o)) {
            print_usage();
            return 0;
        }
#if defined(_OPENMP)
        if (o.threads > 0) omp_set_num_threads(o.threads);
        const int threads = omp_get_max_threads();
#else
        const int threads = 1;
#endif

        auto t0 = std::chrono::steady_clock::now();
        nlohmann::json scene_json = core::SceneLoader::read_json(o.scene);
        core::Scene scene = core::SceneLoader::load_scene(scene_json);
        if (!scene_json.contains("camera")) throw std::runtime_error("scene has no \"camera\"");
        core::Camera camera = core::SceneLoader::load_camera(
            scene_json["camera"], static_cast<float>(o.width) / static_cast<float>(o.height));

        scene.build_acceleration(o.bvh_split == "middle" ? acceleration::BVH::SplitMethod::Middle
                                                         : acceleration::BVH::SplitMethod::SAH);
        scene.set_use_bvh(o.use_bvh);
        auto t1 = std::chrono::steady_clock::now();

        const auto& st = scene.bvh().stats();
        std::cout << "scene " << o.scene << ": " << scene.object_count() << " objects, "
                  << scene.lights().size() << " lights; BVH "
                  << (scene.using_bvh() ? o.bvh_split : std::string("off")) << " (" << st.primitives
                  << " prims, " << st.nodes << " nodes, depth " << st.max_depth << ")\n"
                  << "render " << o.width << "x" << o.height << ", " << o.spp << " spp, depth "
                  << o.depth << ", RR " << (o.rr_start > 0 ? std::to_string(o.rr_start) : "off")
                  << ", threads " << threads << std::endl;

        double render_seconds = 0.0;
        if (o.snapshots.empty()) {
            rendering::Renderer renderer(o.width, o.height, o.spp, o.depth);
            renderer.set_seed(o.seed);
            renderer.integrator().set_russian_roulette_start(o.rr_start);
            auto r0 = std::chrono::steady_clock::now();
            renderer.render(camera, scene);
            render_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - r0).count();
            if (!renderer.framebuffer().save(o.out)) return 1;
        } else {
            rendering::Integrator integrator;
            integrator.set_russian_roulette_start(o.rr_start);
            rendering::Framebuffer fb(o.width, o.height);
            fb.clear();
            auto r0 = std::chrono::steady_clock::now();
            render_progressive(o, camera, scene, integrator, fb);
            render_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - r0).count();
            if (!fb.save(o.out)) return 1;
        }

        std::printf("load+build %.3f s, render %.2f s, wrote %s\n",
                    std::chrono::duration<double>(t1 - t0).count(), render_seconds, o.out.c_str());
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        print_usage();
        return 1;
    }
}
