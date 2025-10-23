#pragma once
#include "scene.h"
#include "../materials/lambertian.h"
#include "../materials/metal.h"
#include "../materials/dielectric.h"
#include "../materials/emissive.h"
#include "../geometry/sphere.h"
#include "light.h"
#include <string>

namespace raytracer {
namespace core {

class SceneBuilder {
public:
    static Scene create_cornell_box();
    static Scene create_test_materials();
    static Scene create_basic_scene();
};

} // namespace core
} // namespace raytracer
