#pragma once

#include "../geometry/primitive.h"
#include "../acceleration/bvh.h"
#include "light.h"
#include <vector>
#include <memory>

namespace raytracer {
namespace core {

class Scene {
public:
    Scene() = default;

    void add(std::shared_ptr<geometry::Primitive> object);
    void add_light(std::shared_ptr<Light> light);
    void clear();

    /**
     * @brief Builds the BVH over all bounded primitives
     *
     * Must be called again after add(); until then hit() falls back to
     * testing every object. Unbounded primitives (planes) are always
     * tested linearly.
     */
    void build_acceleration(acceleration::BVH::SplitMethod method = acceleration::BVH::SplitMethod::SAH);

    /// Enables/disables use of the BVH (for benchmarking and testing)
    void set_use_bvh(bool use) { use_bvh_ = use; }
    bool using_bvh() const { return use_bvh_ && bvh_ready_; }

    bool hit(const Ray& ray, float t_min, float t_max, geometry::HitRecord& rec) const;
    /// Reference implementation: tests every object, O(n) per ray
    bool hit_brute_force(const Ray& ray, float t_min, float t_max, geometry::HitRecord& rec) const;

    const std::vector<std::shared_ptr<Light>>& lights() const { return lights_; }

    /**
     * @brief Background radiance for rays that leave the scene
     *
     * Vertical gradient from 'bottom' (looking down) to 'top' (looking up).
     * Default is the original white-to-sky-blue gradient.
     */
    void set_background(const Color& top, const Color& bottom) {
        background_top_ = top;
        background_bottom_ = bottom;
    }
    Color background(const Vec3& direction) const {
        float t = 0.5f * (glm::normalize(direction).y + 1.0f);
        return (1.0f - t) * background_bottom_ + t * background_top_;
    }
    size_t object_count() const { return objects_.size(); }
    const acceleration::BVH& bvh() const { return bvh_; }

private:
    std::vector<std::shared_ptr<geometry::Primitive>> objects_;
    std::vector<std::shared_ptr<geometry::Primitive>> unbounded_;  // planes etc., outside the BVH
    std::vector<std::shared_ptr<Light>> lights_;
    acceleration::BVH bvh_;
    bool bvh_ready_ = false;
    bool use_bvh_ = true;
    Color background_top_ = Color(0.5f, 0.7f, 1.0f);
    Color background_bottom_ = Color(1.0f, 1.0f, 1.0f);
};

} // namespace core
} // namespace raytracer
