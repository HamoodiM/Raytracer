#include <iostream>
#include <memory>
#include <chrono>
#include <string>
#include <GLFW/glfw3.h>

#include "common.h"
#include "core/scene_gallery.h"
#include "core/scene_loader.h"
#include "rendering/progressive_renderer.h"
#include "viewer/window.h"
#include "viewer/controller.h"
#include "viewer/camera_controller.h"

using namespace raytracer;

void print_usage() {
    std::cout << "=== RayTracer Phase 2 - Interactive Viewer ===" << std::endl;
    std::cout << "Usage: ./raytracer [scene_index]" << std::endl;
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
}

int main(int argc, char* argv[]) {
    const int image_width = 800;
    const int image_height = 600;
    const int max_depth = 50;
    const int target_samples = 100;  // Progressive rendering target
    
    // Parse command line arguments
    int scene_index = 0;
    if (argc > 1) {
        try {
            scene_index = std::stoi(argv[1]);
        } catch (const std::exception& e) {
            std::cerr << "Invalid scene index: " << argv[1] << std::endl;
            print_usage();
            return -1;
        }
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
        return -1;
    }
    
    // Create input controller
    auto controller = std::make_shared<viewer::Controller>(window.get_handle());
    
    // Create camera controller
    auto camera_ptr = std::make_shared<core::Camera>(camera);
    auto camera_controller = std::make_shared<viewer::CameraController>(camera_ptr, controller);
    
    // Set initial camera state for reset functionality
    camera_controller->set_initial_state(camera.position(), camera.look_at(), Vec3(0, 1, 0));
    
    // Create progressive renderer
    rendering::ProgressiveRenderer progressive_renderer(image_width, image_height, max_depth);
    progressive_renderer.set_target_samples(target_samples);
    
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
    
    // Frame timing
    auto last_time = std::chrono::high_resolution_clock::now();
    int frame_count = 0;
    auto fps_start_time = last_time;
    
    // Track camera movement for progressive render reset
    Point3 last_camera_pos = camera_ptr->position();
    Point3 last_camera_look = camera_ptr->look_at();
    
    // Main loop
    while (!window.should_close()) {
        auto current_time = std::chrono::high_resolution_clock::now();
        float delta_time = std::chrono::duration<float>(current_time - last_time).count();
        last_time = current_time;
        
        // Update input
        controller->update();
        camera_controller->update(delta_time);
        
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
        bool still_rendering = progressive_renderer.render_sample(*camera_ptr, scene);
        
        // Update window display
        window.display_framebuffer(progressive_renderer.framebuffer());
        
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
