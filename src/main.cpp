#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <GLFW/glfw3.h>

#include "common.h"
#include "core/scene_gallery.h"
#include "core/scene_loader.h"
#include "rendering/image_io.h"
#include "rendering/progressive_renderer.h"
#include "viewer/window.h"
#include "viewer/controller.h"
#include "viewer/camera_controller.h"

using namespace raytracer;

void print_usage() {
    std::cout << "=== RayTracer Phase 2 - Interactive Viewer ===" << std::endl;
    std::cout << "Usage: ./raytracer [scene_index] [--selftest-frames N] [--selftest-out FILE.png]"
              << " [--selftest-hold-ms MS]" << std::endl;
    std::cout << "Available scenes:" << std::endl;

    const auto& scenes = core::SceneGallery::get_scenes();
    for (size_t i = 0; i < scenes.size(); ++i) {
        std::cout << "  " << i << " - " << scenes[i].name << std::endl;
        std::cout << "      " << scenes[i].description << std::endl;
        std::cout << "      Features: " << scenes[i].features << std::endl;
    }

    std::cout << "\nControls:" << std::endl;
    std::cout << "  WASD       - Move camera (forward/back, left/right)" << std::endl;
    std::cout << "  Q/E        - Move camera up/down" << std::endl;
    std::cout << "  Right Mouse - Rotate camera (hold and drag)" << std::endl;
    std::cout << "  R          - Reset camera to initial position" << std::endl;
    std::cout << "  P          - Save current frame as image" << std::endl;
    std::cout << "  ESC        - Quit" << std::endl;
    std::cout << "\nSelf-test: render N passes in the window (the camera is moved once halfway"
              << " to check that accumulation resets), read the window back with glReadPixels,"
              << " compare it with the CPU image, save the read-back as a PNG, and exit 0 on a"
              << " match, 1 otherwise." << std::endl;
}

namespace {

struct SelftestResult {
    bool pass = false;
    std::string message;
};

// Compares what the window shows with the CPU image the renderer produced.
SelftestResult compare_window_to_cpu(const std::vector<uint8_t>& screen, int screen_w, int screen_h,
                                     const std::vector<uint8_t>& cpu, int cpu_w, int cpu_h) {
    SelftestResult result;
    if (screen_w != cpu_w || screen_h != cpu_h) {
        result.message = "window drawable is " + std::to_string(screen_w) + "x" +
                         std::to_string(screen_h) + ", image is " + std::to_string(cpu_w) + "x" +
                         std::to_string(cpu_h) + " (scaled display; cannot compare 1:1)";
        return result;
    }

    // The reference must have content, or "black == black" would prove nothing.
    const size_t pixels = static_cast<size_t>(cpu_w) * cpu_h;
    size_t lit = 0;
    for (size_t i = 0; i < pixels; ++i) {
        if (cpu[3 * i] > 8 || cpu[3 * i + 1] > 8 || cpu[3 * i + 2] > 8) ++lit;
    }

    // glDrawPixels of 8-bit RGB into an 8-bit RGB drawable should be exact;
    // allow 2 levels per channel for drivers that dither or round differently.
    const int tolerance = 2;
    size_t mismatched = 0;
    int max_diff = 0;
    double sum_diff = 0.0;
    for (size_t i = 0; i < pixels; ++i) {
        int pixel_diff = 0;
        for (int c = 0; c < 3; ++c) {
            int d = std::abs(static_cast<int>(screen[3 * i + c]) - static_cast<int>(cpu[3 * i + c]));
            pixel_diff = std::max(pixel_diff, d);
            sum_diff += d;
        }
        max_diff = std::max(max_diff, pixel_diff);
        if (pixel_diff > tolerance) ++mismatched;
    }
    const double mean_diff = sum_diff / (3.0 * pixels);
    const double lit_fraction = static_cast<double>(lit) / pixels;

    result.message = std::to_string(cpu_w) + "x" + std::to_string(cpu_h) +
                     ", lit pixels " + std::to_string(lit_fraction * 100.0) + "%" +
                     ", mismatched pixels (diff > " + std::to_string(tolerance) + ") " +
                     std::to_string(mismatched) + ", max channel diff " + std::to_string(max_diff) +
                     ", mean channel diff " + std::to_string(mean_diff);
    result.pass = lit_fraction > 0.05 && mismatched == 0;
    if (lit_fraction <= 0.05) result.message += " (reference image is nearly black)";
    return result;
}

} // namespace

int main(int argc, char* argv[]) {
    const int image_width = 800;
    const int image_height = 600;
    const int max_depth = 50;
    const int target_samples = 100;  // Progressive rendering target

    // Parse command line arguments
    int scene_index = 0;
    int selftest_frames = 0;  // 0 = normal interactive run
    std::string selftest_out = "viewer_selftest.png";
    int selftest_hold_ms = 0;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        try {
            if (arg == "--selftest-frames" && i + 1 < argc) {
                selftest_frames = std::stoi(argv[++i]);
            } else if (arg == "--selftest-out" && i + 1 < argc) {
                selftest_out = argv[++i];
            } else if (arg == "--selftest-hold-ms" && i + 1 < argc) {
                selftest_hold_ms = std::stoi(argv[++i]);
            } else if (arg == "-h" || arg == "--help") {
                print_usage();
                return 0;
            } else {
                scene_index = std::stoi(arg);
            }
        } catch (const std::exception&) {
            std::cerr << "Invalid argument: " << arg << std::endl;
            print_usage();
            return -1;
        }
    }
    const bool selftest = selftest_frames > 0;
    if (selftest && selftest_frames < 2) {
        std::cerr << "--selftest-frames needs at least 2 (the camera moves halfway)" << std::endl;
        return -1;
    }

    // Validate scene index
    if (scene_index < 0 || scene_index >= static_cast<int>(core::SceneGallery::scene_count())) {
        std::cerr << "Scene index out of range. Available scenes: 0-"
                  << (core::SceneGallery::scene_count() - 1) << std::endl;
        print_usage();
        return -1;
    }

    // Load selected scene
    auto [scene, camera] = core::SceneGallery::load_scene(scene_index);
    const auto& scene_info = core::SceneGallery::get_scenes()[scene_index];

    // Create window
    std::string window_title = "RayTracer Phase 2 - " + scene_info.name;
    viewer::Window window(image_width, image_height, window_title);
    if (!window.get_handle()) {
        std::cerr << "Failed to create window" << std::endl;
        return selftest ? 1 : -1;
    }
    std::cout << "OpenGL: " << reinterpret_cast<const char*>(glGetString(GL_VERSION))
              << " (" << reinterpret_cast<const char*>(glGetString(GL_RENDERER)) << ")" << std::endl;

    // Create input controller
    auto controller = std::make_shared<viewer::Controller>(window.get_handle());

    // Create camera controller
    auto camera_ptr = std::make_shared<core::Camera>(camera);
    auto camera_controller = std::make_shared<viewer::CameraController>(camera_ptr, controller);

    // Set initial camera state for reset functionality
    camera_controller->set_initial_state(camera.position(), camera.look_at(), Vec3(0, 1, 0));

    // Create progressive renderer
    rendering::ProgressiveRenderer progressive_renderer(image_width, image_height, max_depth);
    progressive_renderer.set_target_samples(selftest ? selftest_frames : target_samples);

    // Print startup information
    std::cout << "\n=== RayTracer Phase 2 - Interactive Viewer ===" << std::endl;
    std::cout << "Scene: " << scene_info.name << std::endl;
    std::cout << "Description: " << scene_info.description << std::endl;
    std::cout << "Features: " << scene_info.features << std::endl;
    std::cout << "Resolution: " << image_width << "x" << image_height << std::endl;
    std::cout << "Target samples: " << target_samples << std::endl;
    std::cout << "\nControls:" << std::endl;
    std::cout << "  WASD       - Move camera" << std::endl;
    std::cout << "  Q/E        - Move up/down" << std::endl;
    std::cout << "  Right Mouse - Rotate camera" << std::endl;
    std::cout << "  R          - Reset camera" << std::endl;
    std::cout << "  P          - Save image" << std::endl;
    std::cout << "  ESC        - Quit" << std::endl;
    std::cout << "\nStarting progressive rendering..." << std::endl;
    if (selftest) {
        std::cout << "Self-test: " << selftest_frames << " frames, camera moves after frame "
                  << selftest_frames / 2 << std::endl;
    }

    // Frame timing
    auto last_time = std::chrono::high_resolution_clock::now();
    int frame_count = 0;
    auto fps_start_time = last_time;

    // Track camera movement for progressive render reset
    Point3 last_camera_pos = camera_ptr->position();
    Point3 last_camera_look = camera_ptr->look_at();

    bool selftest_reset_seen = false;

    // Main loop
    while (!window.should_close()) {
        auto current_time = std::chrono::high_resolution_clock::now();
        float delta_time = std::chrono::duration<float>(current_time - last_time).count();
        last_time = current_time;

        // Update input, then clear the mouse delta that was just used
        camera_controller->update(delta_time);
        controller->update();

        // Self-test: move the camera halfway through, as a key press would
        if (selftest && frame_count == selftest_frames / 2) {
            camera_ptr->translate(Vec3(0.25f, 0.0f, 0.0f));
        }

        // Check if camera moved (for progressive render reset)
        Point3 current_camera_pos = camera_ptr->position();
        Point3 current_camera_look = camera_ptr->look_at();

        bool camera_moved = (current_camera_pos != last_camera_pos) ||
                           (current_camera_look != last_camera_look);

        if (camera_moved) {
            progressive_renderer.reset();
            last_camera_pos = current_camera_pos;
            last_camera_look = current_camera_look;
        }

        // Render one sample iteration
        progressive_renderer.render_sample(*camera_ptr, scene);
        if (selftest && camera_moved) {
            selftest_reset_seen = progressive_renderer.sample_count() == 1;
        }

        // Draw the accumulated image into the back buffer
        window.display_framebuffer(progressive_renderer.framebuffer());

        if (selftest && frame_count + 1 == selftest_frames) {
            // Last self-test frame: compare the back buffer with the CPU image
            int screen_w = 0, screen_h = 0;
            std::vector<uint8_t> screen = window.read_back_rgb8(screen_w, screen_h);
            std::vector<uint8_t> cpu = progressive_renderer.framebuffer().to_rgb8();
            SelftestResult result = compare_window_to_cpu(screen, screen_w, screen_h,
                                                          cpu, image_width, image_height);

            const int expected_samples = selftest_frames - selftest_frames / 2;
            const bool samples_ok = progressive_renderer.sample_count() == expected_samples;
            bool saved = rendering::write_image(selftest_out, screen_w, screen_h, screen);

            std::cout << "Self-test: accumulation reset on camera move: "
                      << (selftest_reset_seen ? "yes" : "NO") << std::endl;
            std::cout << "Self-test: samples after the move " << progressive_renderer.sample_count()
                      << " (expected " << expected_samples << ")" << std::endl;
            std::cout << "Self-test: window vs CPU image: " << result.message << std::endl;
            std::cout << "Self-test: window read-back " << (saved ? "saved to " : "NOT saved to ")
                      << selftest_out << std::endl;

            const bool pass = result.pass && selftest_reset_seen && samples_ok && saved;
            std::cout << (pass ? "SELFTEST PASS" : "SELFTEST FAIL") << std::endl;

            // Optionally keep the final image on screen (e.g. for an external screenshot)
            window.swap_buffers();
            auto hold_until = std::chrono::steady_clock::now() + std::chrono::milliseconds(selftest_hold_ms);
            while (std::chrono::steady_clock::now() < hold_until && !window.should_close()) {
                window.poll_events();
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
            return pass ? 0 : 1;
        }

        // Handle special keys
        if (controller->is_key_pressed(GLFW_KEY_ESCAPE)) {
            break;
        }

        if (controller->is_key_pressed(GLFW_KEY_P)) {
            // Save current frame
            std::string filename = "screenshot_" + std::to_string(frame_count) + ".ppm";
            progressive_renderer.framebuffer().save_ppm(filename);
            std::cout << "Screenshot saved: " << filename << std::endl;
        }

        if (controller->is_key_pressed(GLFW_KEY_R)) {
            // Reset camera
            camera_controller->reset_camera();
            progressive_renderer.reset();
            std::cout << "Camera reset to initial position" << std::endl;
        }

        // Update window
        window.swap_buffers();
        window.poll_events();

        // Calculate FPS
        frame_count++;
        if (frame_count % 60 == 0) {
            auto fps_time = std::chrono::high_resolution_clock::now();
            float fps = 60.0f / std::chrono::duration<float>(fps_time - fps_start_time).count();
            fps_start_time = fps_time;

            std::cout << "FPS: " << static_cast<int>(fps)
                      << " | Samples: " << progressive_renderer.sample_count()
                      << "/" << target_samples << std::endl;
        }
    }

    // Final save
    std::string final_filename = "final_render_" + scene_info.name + ".ppm";
    progressive_renderer.framebuffer().save_ppm(final_filename);

    std::cout << "\n=== RENDER COMPLETE ===" << std::endl;
    std::cout << "Final render saved: " << final_filename << std::endl;
    std::cout << "Total samples: " << progressive_renderer.sample_count() << std::endl;

    return 0;
}
