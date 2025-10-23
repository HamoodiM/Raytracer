#pragma once

#include "ray.h"
#include <glm/glm.hpp>

namespace raytracer {
namespace core {

class Camera {
public:
    Camera() = default;
    Camera(const Point3& lookfrom, const Point3& lookat, const Vec3& vup,
           float vfov, float aspect_ratio, float aperture = 0.0f, float focus_dist = 1.0f);

    Ray get_ray(float u, float v) const;
    
    /**
     * @brief Updates camera position and orientation
     * 
     * @param lookfrom New camera position
     * @param lookat New look-at target
     * @param vup Up vector
     */
    void update(const Point3& lookfrom, const Point3& lookat, const Vec3& vup);
    
    /**
     * @brief Translates the camera in world space
     * 
     * @param delta Translation vector
     */
    void translate(const Vec3& delta);
    
    /**
     * @brief Rotates the camera around the look-at point
     * 
     * @param yaw Yaw angle in radians
     * @param pitch Pitch angle in radians
     */
    void rotate(float yaw, float pitch);
    
    /**
     * @brief Changes the field of view
     * 
     * @param vfov New vertical field of view in degrees
     */
    void set_fov(float vfov);
    
    /**
     * @brief Gets the current camera position
     * 
     * @return Camera position
     */
    Point3 position() const { return origin_; }
    
    /**
     * @brief Gets the current look-at target
     * 
     * @return Look-at target position
     */
    Point3 look_at() const { return lookat_; }

private:
    Point3 origin_;
    Point3 lookat_;
    Vec3 vup_;
    float vfov_;
    float aspect_ratio_;
    float aperture_;
    float focus_dist_;
    
    Point3 lower_left_corner_;
    Vec3 horizontal_;
    Vec3 vertical_;
    Vec3 u_, v_, w_;
    float lens_radius_;
    
    /**
     * @brief Rebuilds camera vectors after parameter changes
     */
    void rebuild();
};

} // namespace core
} // namespace raytracer
