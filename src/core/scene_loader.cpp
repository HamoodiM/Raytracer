/**
 * @file scene_loader.cpp
 * @brief Implementation of JSON scene file parser and loader
 */

#include "scene_loader.h"
#include "../geometry/sphere.h"
#include "../geometry/plane.h"
#include "../geometry/triangle.h"
#include "../geometry/mesh_normals.h"
#include "../materials/lambertian.h"
#include "../materials/textured_lambertian.h"
#include "../materials/metal.h"
#include "../materials/dielectric.h"
#include "../materials/emissive.h"
#include "../textures/image_texture.h"
#include "../textures/checker_texture.h"
#include "../textures/normal_map.h"
#include "../common.h"
#include <cstdio>
#include <array>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <iostream>
#include <stdexcept>

// JSON library is included in the header

namespace raytracer {
namespace core {

nlohmann::json SceneLoader::read_json(const std::string& filename) {
    // C stdio instead of <fstream>, so this also builds for wasm32-wasi
    std::FILE* file = std::fopen(filename.c_str(), "rb");
    if (!file) {
        throw std::runtime_error("Failed to open scene file: " + filename);
    }
    std::string text;
    char buffer[4096];
    size_t n;
    while ((n = std::fread(buffer, 1, sizeof(buffer), file)) > 0) text.append(buffer, n);
    std::fclose(file);

    try {
        return nlohmann::json::parse(text);
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error("Failed to parse JSON: " + std::string(e.what()));
    }
}

Scene SceneLoader::load_scene(const std::string& filename) {
    return load_scene(read_json(filename));
}

Scene SceneLoader::load_scene(const nlohmann::json& scene_json) {
    Scene scene;
    
    // Optional background: a colour, or {"top": [...], "bottom": [...]}
    if (scene_json.contains("background")) {
        const auto& bg = scene_json["background"];
        if (bg.is_array()) {
            Color c = parse_color(bg);
            scene.set_background(c, c);
        } else {
            scene.set_background(parse_color(bg["top"]), parse_color(bg["bottom"]));
        }
    }

    // Load objects
    if (scene_json.contains("objects")) {
        for (const auto& object_json : scene_json["objects"]) {
            if (object_json.value("type", std::string()) == "obj") {
                load_obj(object_json, scene);  // adds one Triangle per face
                continue;
            }
            auto primitive = create_primitive(object_json);
            scene.add(primitive);
        }
    }
    
    // Load lights
    if (scene_json.contains("lights")) {
        for (const auto& light_json : scene_json["lights"]) {
            auto light = create_light(light_json);
            scene.add_light(light);

            // "visible": true gives an area light real geometry (two emissive
            // triangles), so it shows up in the camera, mirrors and glass.
            // That geometry is flagged as light-sampled so the integrator does
            // not count it a second time after next-event estimation.
            if (light_json.value("type", std::string()) == "area" && light_json.value("visible", false)) {
                Point3 c = parse_vec3(light_json["center"]);
                Vec3 u = parse_vec3(light_json["u"]) * 0.5f;
                Vec3 v = parse_vec3(light_json["v"]) * 0.5f;
                auto emitter = std::make_shared<materials::Emissive>(
                    parse_color(light_json["emission"]), /*sampled_by_lights=*/true);
                Point3 p00 = c - u - v, p10 = c + u - v, p11 = c + u + v, p01 = c - u + v;
                scene.add(std::make_shared<geometry::Triangle>(p00, p10, p11, emitter));
                scene.add(std::make_shared<geometry::Triangle>(p00, p11, p01, emitter));
            }
        }
    }
    
    scene.build_acceleration();
    return scene;
}

Camera SceneLoader::load_camera(const nlohmann::json& camera_json, float aspect_ratio_override) {
    Point3 position = parse_vec3(camera_json["position"]);
    Point3 look_at = parse_vec3(camera_json["look_at"]);
    Vec3 vup = parse_vec3(camera_json.value("vup", nlohmann::json{0, 1, 0}));
    float vfov = camera_json.value("vfov", 40.0f);
    float aspect_ratio = aspect_ratio_override > 0.0f ? aspect_ratio_override
                                                      : camera_json.value("aspect_ratio", 16.0f/9.0f);
    float aperture = camera_json.value("aperture", 0.0f);
    float focus_distance = camera_json.value("focus_distance", 1.0f);
    
    return Camera(position, look_at, vup, vfov, aspect_ratio, aperture, focus_distance);
}

std::shared_ptr<geometry::Primitive> SceneLoader::create_primitive(const nlohmann::json& object_json) {
    std::string type = object_json["type"];
    auto material = create_material(object_json["material"]);
    
    if (type == "sphere") {
        Point3 center = parse_vec3(object_json["center"]);
        float radius = object_json["radius"];
        return std::make_shared<geometry::Sphere>(center, radius, material);
    }
    else if (type == "plane") {
        Point3 point = parse_vec3(object_json["point"]);
        Vec3 normal = parse_vec3(object_json["normal"]);
        return std::make_shared<geometry::Plane>(point, normal, material);
    }
    else if (type == "triangle") {
        Point3 v0 = parse_vec3(object_json["v0"]);
        Point3 v1 = parse_vec3(object_json["v1"]);
        Point3 v2 = parse_vec3(object_json["v2"]);
        return std::make_shared<geometry::Triangle>(v0, v1, v2, material);
    }
    else {
        throw std::runtime_error("Unknown primitive type: " + type);
    }
}

std::shared_ptr<materials::Material> SceneLoader::create_material(const nlohmann::json& material_json) {
    std::string type = material_json["type"];
    
    if (type == "lambertian") {
        if (material_json.contains("texture")) {
            auto texture = create_texture(material_json["texture"]);
            return std::make_shared<materials::Lambertian>(texture);
        } else {
            Color albedo = parse_color(material_json["albedo"]);
            return std::make_shared<materials::Lambertian>(albedo);
        }
    }
    else if (type == "textured_lambertian") {
        auto albedo_texture = create_texture(material_json["albedo_texture"]);
        std::shared_ptr<textures::NormalMap> normal_map = nullptr;
        if (material_json.contains("normal_map")) {
            auto normal_texture = create_texture(material_json["normal_map"]);
            float strength = material_json.value("normal_strength", 1.0f);
            normal_map = std::make_shared<textures::NormalMap>(normal_texture, strength);
        }
        return std::make_shared<materials::TexturedLambertian>(albedo_texture, normal_map);
    }
    else if (type == "metal") {
        Color albedo = parse_color(material_json["albedo"]);
        float fuzz = material_json.value("fuzz", 0.0f);
        return std::make_shared<materials::Metal>(albedo, fuzz);
    }
    else if (type == "dielectric") {
        float ir = material_json.value("ir", 1.5f);
        return std::make_shared<materials::Dielectric>(ir);
    }
    else if (type == "emissive") {
        Color emission = parse_color(material_json["emission"]);
        return std::make_shared<materials::Emissive>(emission);
    }
    else {
        throw std::runtime_error("Unknown material type: " + type);
    }
}

std::shared_ptr<textures::Texture> SceneLoader::create_texture(const nlohmann::json& texture_json) {
    if (texture_json.is_string()) {
        // Image texture from filename
        std::string filename = texture_json;
        return std::make_shared<textures::ImageTexture>(filename);
    }
    else if (texture_json.is_object()) {
        std::string type = texture_json["type"];
        
        if (type == "checker") {
            Color color1 = parse_color(texture_json["color1"]);
            Color color2 = parse_color(texture_json["color2"]);
            float frequency = texture_json.value("frequency", 10.0f);
            return std::make_shared<textures::CheckerTexture>(color1, color2, frequency);
        }
        else {
            throw std::runtime_error("Unknown texture type: " + type);
        }
    }
    else {
        throw std::runtime_error("Invalid texture configuration");
    }
}

std::shared_ptr<Light> SceneLoader::create_light(const nlohmann::json& light_json) {
    std::string type = light_json["type"];
    
    if (type == "point") {
        Point3 position = parse_vec3(light_json["position"]);
        Color intensity = parse_color(light_json["intensity"]);
        return std::make_shared<PointLight>(position, intensity);
    }
    else if (type == "area") {
        Point3 center = parse_vec3(light_json["center"]);
        Vec3 u = parse_vec3(light_json["u"]);
        Vec3 v = parse_vec3(light_json["v"]);
        Color emission = parse_color(light_json["emission"]);
        return std::make_shared<AreaLight>(center, u, v, emission);
    }
    else {
        throw std::runtime_error("Unknown light type: " + type);
    }
}

void SceneLoader::load_obj(const nlohmann::json& object_json, Scene& scene) {
    // Minimal Wavefront OBJ reader: "v", "vn" and "f" lines. Polygons are
    // fan-triangulated; face corners may be "v", "v/vt", "v//vn" or
    // "v/vt/vn", and negative (relative) indices are accepted. UVs, groups
    // and materials are ignored.
    //
    // "smooth": true gives each triangle per-vertex normals: the file's "vn"
    // normals if every face corner has one, otherwise area-weighted averages
    // of the faces around each vertex. Default false (flat shading).
    const std::string filename = object_json["file"];
    const float scale = object_json.value("scale", 1.0f);
    const Vec3 translate =
        object_json.contains("translate") ? parse_vec3(object_json["translate"]) : Vec3(0.0f);
    const bool smooth = object_json.value("smooth", false);
    auto material = create_material(object_json["material"]);

    std::FILE* file = std::fopen(filename.c_str(), "rb");
    if (!file) throw std::runtime_error("Failed to open OBJ file: " + filename);

    std::vector<Point3> vertices;
    std::vector<Vec3> file_normals;
    std::vector<std::array<int, 3>> faces;          // vertex indices per triangle
    std::vector<std::array<int, 3>> face_normals;   // "vn" indices per triangle (-1 = none)
    bool every_corner_has_vn = true;

    auto resolve = [](int i, size_t count) {  // 1-based or negative OBJ index -> 0-based
        return i < 0 ? static_cast<int>(count) + i : i - 1;
    };

    char line[1024];
    while (std::fgets(line, sizeof(line), file)) {
        if (line[0] == 'v' && line[1] == ' ') {
            float x, y, z;
            if (std::sscanf(line + 2, "%f %f %f", &x, &y, &z) == 3)
                vertices.push_back(Point3(x, y, z) * scale + translate);
        } else if (line[0] == 'v' && line[1] == 'n' && line[2] == ' ') {
            float x, y, z;
            if (std::sscanf(line + 3, "%f %f %f", &x, &y, &z) == 3)
                file_normals.push_back(Vec3(x, y, z));  // uniform scale: direction unchanged
        } else if (line[0] == 'f' && line[1] == ' ') {
            std::vector<int> vi, ni;
            const char* p = line + 2;
            const char* separators = " \t\r\n";
            while (*p) {
                p += std::strspn(p, separators);
                if (*p == '\0') break;
                size_t token_len = std::strcspn(p, separators);
                std::string token(p, token_len);
                p += token_len;

                int v = 0, t = 0, n = 0;
                if (std::sscanf(token.c_str(), "%d/%d/%d", &v, &t, &n) == 3 ||
                    std::sscanf(token.c_str(), "%d//%d", &v, &n) == 2) {
                    ni.push_back(resolve(n, file_normals.size()));
                } else {
                    v = std::atoi(token.c_str());  // "v" or "v/vt"
                    ni.push_back(-1);
                    every_corner_has_vn = false;
                }
                vi.push_back(resolve(v, vertices.size()));
            }
            const int nv = static_cast<int>(vertices.size());
            const int nn = static_cast<int>(file_normals.size());
            for (size_t k = 1; k + 1 < vi.size(); ++k) {
                std::array<int, 3> f = {vi[0], vi[k], vi[k + 1]};
                std::array<int, 3> fn = {ni[0], ni[k], ni[k + 1]};
                for (int c = 0; c < 3; ++c) {
                    if (f[c] < 0 || f[c] >= nv || fn[c] >= nn) {
                        std::fclose(file);
                        throw std::runtime_error("Bad face index in " + filename);
                    }
                }
                faces.push_back(f);
                face_normals.push_back(fn);
            }
        }
    }
    std::fclose(file);

    const bool use_file_normals = smooth && every_corner_has_vn && !file_normals.empty();
    std::vector<Vec3> vertex_normals;
    if (smooth && !use_file_normals) vertex_normals = geometry::compute_vertex_normals(vertices, faces);

    for (size_t i = 0; i < faces.size(); ++i) {
        const auto& f = faces[i];
        if (!smooth) {
            scene.add(std::make_shared<geometry::Triangle>(vertices[f[0]], vertices[f[1]], vertices[f[2]],
                                                          material));
            continue;
        }
        Vec3 n[3];
        for (int c = 0; c < 3; ++c)
            n[c] = use_file_normals ? glm::normalize(file_normals[face_normals[i][c]]) : vertex_normals[f[c]];
        scene.add(std::make_shared<geometry::Triangle>(vertices[f[0]], vertices[f[1]], vertices[f[2]],
                                                      n[0], n[1], n[2], material));
    }

    std::cout << "Loaded mesh: " << filename << " (" << vertices.size() << " vertices, "
              << faces.size() << " triangles, "
              << (!smooth ? "flat" : use_file_normals ? "smooth, file normals" : "smooth, computed normals")
              << ")" << std::endl;
}

Vec3 SceneLoader::parse_vec3(const nlohmann::json& json_array) {
    if (!json_array.is_array() || json_array.size() != 3) {
        throw std::runtime_error("Invalid Vec3 format - expected [x, y, z] array");
    }
    
    return Vec3(json_array[0], json_array[1], json_array[2]);
}

Color SceneLoader::parse_color(const nlohmann::json& json_array) {
    if (!json_array.is_array() || json_array.size() != 3) {
        throw std::runtime_error("Invalid Color format - expected [r, g, b] array");
    }
    
    return Color(json_array[0], json_array[1], json_array[2]);
}

} // namespace core
} // namespace raytracer
