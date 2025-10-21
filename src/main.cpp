#include <iostream>
#include <memory>
#include <glm/glm.hpp>

#include "common.h"
#include "core/camera.h"
#include "core/scene.h"
#include "geometry/sphere.h"
#include "materials/lambertian.h"
#include "rendering/renderer.h"
#include "viewer/window.h"
#include "viewer/controller.h"

using namespace raytracer;

int main() {
    // Image dimensions
    const int image_width = 800;
    const int image_height = 600;
    const int samples_per_pixel = 100;
    const int max_depth = 50;

    // Create window
    viewer::Window window(image_width, image_height, "RayTracer");
    if (!window.get_handle()) {
        std::cerr << "Failed to create window" << std::endl;
        return -1;
    }

    // Create scene
    core::Scene scene;

    // Materials
    auto material_ground = std::make_shared<materials::Lambertian>(Color(0.5f, 0.5f, 0.5f));
    auto material_center = std::make_shared<materials::Lambertian>(Color(0.7f, 0.3f, 0.3f));
    auto material_left   = std::make_shared<materials::Lambertian>(Color(0.3f, 0.3f, 0.7f));
    auto material_right  = std::make_shared<materials::Lambertian>(Color(0.3f, 0.7f, 0.3f));

    // Objects
    scene.add(std::make_shared<geometry::Sphere>(Point3( 0.0f, -100.5f, -1.0f), 100.0f, material_ground));
    scene.add(std::make_shared<geometry::Sphere>(Point3( 0.0f,    0.0f, -1.0f),   0.5f, material_center));
    scene.add(std::make_shared<geometry::Sphere>(Point3(-1.0f,    0.0f, -1.0f),   0.5f, material_left));
    scene.add(std::make_shared<geometry::Sphere>(Point3( 1.0f,    0.0f, -1.0f),   0.5f, material_right));

    // Camera
    Point3 lookfrom(3, 3, 2);
    Point3 lookat(0, 0, -1);
    Vec3 vup(0, 1, 0);
    auto dist_to_focus = glm::length(lookfrom - lookat);
    auto aperture = 2.0f;

    core::Camera camera(lookfrom, lookat, vup, 20, static_cast<float>(image_width) / image_height, aperture, dist_to_focus);

    // Renderer
    rendering::Renderer renderer(image_width, image_height, samples_per_pixel, max_depth);

    // Render the scene
    renderer.render(camera, scene);

    // Display in window once
    window.display_framebuffer(renderer.framebuffer());
    
    // Keep window open until user closes it
    while (!window.should_close()) {
        window.poll_events();
        window.swap_buffers();
    }

    // Save image
    renderer.framebuffer().save_ppm("output.ppm");

    return 0;
}
