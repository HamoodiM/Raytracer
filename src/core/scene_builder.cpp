#include "scene_builder.h"
#include <memory>

using namespace std;

namespace raytracer {
namespace core {

Scene SceneBuilder::create_cornell_box() {
    Scene scene;
    
    // Materials
    auto red = make_shared<materials::Lambertian>(Color(0.65f, 0.05f, 0.05f));
    auto white = make_shared<materials::Lambertian>(Color(0.73f, 0.73f, 0.73f));
    auto green = make_shared<materials::Lambertian>(Color(0.12f, 0.45f, 0.15f));
    auto light = make_shared<materials::Emissive>(Color(500.0f, 500.0f, 500.0f));
    auto metal = make_shared<materials::Metal>(Color(0.8f, 0.85f, 0.88f), 0.0f);
    auto glass = make_shared<materials::Dielectric>(1.5f);
    
    // Cornell Box walls (using large spheres to approximate planes)
    scene.add(make_shared<geometry::Sphere>(Point3(0, -100.5f, -1), 100.0f, white)); // Floor
    scene.add(make_shared<geometry::Sphere>(Point3(0, 100.5f, -1), 100.0f, white));  // Ceiling
    scene.add(make_shared<geometry::Sphere>(Point3(-100.5f, 0, -1), 100.0f, red));   // Left wall (red)
    scene.add(make_shared<geometry::Sphere>(Point3(100.5f, 0, -1), 100.0f, green));  // Right wall (green)
    scene.add(make_shared<geometry::Sphere>(Point3(0, 0, -101), 100.0f, white));     // Back wall
    
    // Light source (small emissive sphere at ceiling)
    scene.add(make_shared<geometry::Sphere>(Point3(0, 0.9f, -1), 0.2f, light));
    
    // Add area light for direct lighting
    auto area_light = make_shared<AreaLight>(
        Point3(0, 0.9f, -1),          // center
        Vec3(0.3f, 0, 0),              // u direction
        Vec3(0, 0, 0.3f),              // v direction
        Color(500.0f, 500.0f, 500.0f)     // emission
    );
    scene.add_light(area_light);
    
    // Objects
    scene.add(make_shared<geometry::Sphere>(Point3(-0.3f, -0.2f, -1), 0.3f, metal));
    scene.add(make_shared<geometry::Sphere>(Point3(0.3f, -0.2f, -1), 0.3f, glass));
    
    return scene;
}

Scene SceneBuilder::create_test_materials() {
    Scene scene;
    
    auto ground = make_shared<materials::Lambertian>(Color(0.5f, 0.5f, 0.5f));
    auto metal_polished = make_shared<materials::Metal>(Color(0.8f, 0.6f, 0.2f), 0.0f);
    auto metal_rough = make_shared<materials::Metal>(Color(0.8f, 0.8f, 0.8f), 0.5f);
    auto glass = make_shared<materials::Dielectric>(1.5f);
    auto emissive = make_shared<materials::Emissive>(Color(4.0f, 4.0f, 4.0f));
    
    // Ground
    scene.add(make_shared<geometry::Sphere>(Point3(0, -100.5f, -1), 100.0f, ground));
    
    // Test spheres
    scene.add(make_shared<geometry::Sphere>(Point3(-1.2f, 0, -1), 0.5f, metal_polished));
    scene.add(make_shared<geometry::Sphere>(Point3(0, 0, -1), 0.5f, glass));
    scene.add(make_shared<geometry::Sphere>(Point3(1.2f, 0, -1), 0.5f, metal_rough));
    scene.add(make_shared<geometry::Sphere>(Point3(0, 2.0f, -1), 0.5f, emissive));
    
    // Add point light
    auto point_light = make_shared<PointLight>(Point3(0, 2.0f, -1), Color(4.0f, 4.0f, 4.0f));
    scene.add_light(point_light);
    
    return scene;
}

Scene SceneBuilder::create_basic_scene() {
    Scene scene;
    
    auto material_ground = make_shared<materials::Lambertian>(Color(0.5f, 0.5f, 0.5f));
    auto material_center = make_shared<materials::Lambertian>(Color(0.7f, 0.3f, 0.3f));
    auto material_left = make_shared<materials::Lambertian>(Color(0.3f, 0.3f, 0.7f));
    auto material_right = make_shared<materials::Lambertian>(Color(0.3f, 0.7f, 0.3f));
    
    scene.add(make_shared<geometry::Sphere>(Point3(0.0f, -100.5f, -1.0f), 100.0f, material_ground));
    scene.add(make_shared<geometry::Sphere>(Point3(0.0f, 0.0f, -1.0f), 0.5f, material_center));
    scene.add(make_shared<geometry::Sphere>(Point3(-1.0f, 0.0f, -1.0f), 0.5f, material_left));
    scene.add(make_shared<geometry::Sphere>(Point3(1.0f, 0.0f, -1.0f), 0.5f, material_right));
    
    return scene;
}

} // namespace core
} // namespace raytracer
