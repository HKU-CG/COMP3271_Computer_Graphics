#include "Renderer.hpp"
#include <numeric>

namespace a3 {
bool AABB::Hit(const Ray &ray, double t_min, double t_max) const {
    // ===== STUDENT_TASK_BEGIN: assignment3_aabb_hit =====
    // TODO: implement the handout task.
    return false;
    // ===== STUDENT_TASK_END: assignment3_aabb_hit =====
}

// Provided initialization/ownership. Only the recursive split is assessed.
void BVH::Build() {
    nodes.clear();
    indices.resize(primitives.size());
    std::iota(indices.begin(), indices.end(), std::size_t{0});
    if (!indices.empty()) BuildRange(0, indices.size());
}

int BVH::BuildRange(std::size_t begin, std::size_t end) {
    // ===== STUDENT_TASK_BEGIN: assignment3_bvh_build =====
    // TODO: implement the handout task.
    return -1;
    // ===== STUDENT_TASK_END: assignment3_bvh_build =====
}

bool BVH::Hit(const Ray &ray, double lo, double hi, HitRecord &hit, QueryStats *stats) const {
    return !nodes.empty() && HitNode(0, ray, lo, hi, hit, stats);
}

bool BVH::HitNode(
    int index, const Ray &ray, double lo, double hi, HitRecord &hit, QueryStats *stats) const {
    // ===== STUDENT_TASK_BEGIN: assignment3_bvh_hit =====
    // TODO: implement the handout task.
    return false;
    // ===== STUDENT_TASK_END: assignment3_bvh_hit =====
}
} // namespace a3
