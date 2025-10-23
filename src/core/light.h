#pragma once
#include "../common.h"
#include "ray.h"

namespace raytracer {
namespace core {

struct LightSample {
    Point3 position;
    Vec3 direction;
    Color radiance;
    float distance;
    float pdf;
};

class Light {
public:
    virtual ~Light() = default;
    virtual LightSample sample(const Point3& hit_point) const = 0;
    virtual Color radiance() const = 0;
};

class PointLight : public Light {
public:
    PointLight(const Point3& position, const Color& intensity)
        : position_(position), intensity_(intensity) {}
    
    LightSample sample(const Point3& hit_point) const override;
    Color radiance() const override { return intensity_; }

private:
    Point3 position_;
    Color intensity_;
};

class AreaLight : public Light {
public:
    AreaLight(const Point3& center, const Vec3& u, const Vec3& v, const Color& emission)
        : center_(center), u_(u), v_(v), emission_(emission) {}
    
    LightSample sample(const Point3& hit_point) const override;
    Color radiance() const override { return emission_; }

private:
    Point3 center_;
    Vec3 u_, v_;  // Two edges defining the rectangular area
    Color emission_;
};

} // namespace core
} // namespace raytracer
