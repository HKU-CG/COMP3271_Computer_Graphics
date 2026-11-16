#include "Renderer.hpp"
#include <numeric>

namespace a3 {
bool AABB::Hit(const Ray &ray, double t_min, double t_max) const {
    // ===== STUDENT_TASK_BEGIN: assignment3_aabb_hit =====
    // TODO: implement the handout task.
    return false;
    // ===== STUDENT_TASK_END: assignment3_aabb_hit =====
}

// Provided BVH construction and traversal. Only AABB::Hit is a student task.
void BVH::Build() {
    nodes.clear();
    indices.resize(primitives.size());
    std::iota(indices.begin(), indices.end(), std::size_t{0});
    if (!indices.empty()) BuildRange(0, indices.size());
}

int BVH::BuildRange(std::size_t begin, std::size_t end) {
    int index = static_cast<int>(nodes.size());
    nodes.emplace_back();
    AABB bounds, centroids;
    for (std::size_t i = begin; i < end; ++i) {
        AABB b = primitives[indices[i]]->Bounds();
        bounds.Expand(b);
        Vec3 c = b.Centroid();
        centroids.Expand({c, c});
    }
    nodes[index].bounds = bounds;
    if (end - begin <= 4) {
        nodes[index].start = begin;
        nodes[index].count = end - begin;
    } else {
        Vec3 extent = centroids.maximum - centroids.minimum;
        int axis = 0;
        if (extent.y > extent[axis]) axis = 1;
        if (extent.z > extent[axis]) axis = 2;
        std::size_t middle = begin + (end - begin) / 2;
        // Median by index always makes progress, even if all centroids coincide.
        std::nth_element(
            indices.begin() + begin, indices.begin() + middle, indices.begin() + end,
            [&](std::size_t a, std::size_t b) {
                double ca = primitives[a]->Bounds().Centroid()[axis];
                double cb = primitives[b]->Bounds().Centroid()[axis];
                return ca < cb || (ca == cb && primitives[a]->id < primitives[b]->id);
            });
        int left = BuildRange(begin, middle);
        int right = BuildRange(middle, end);
        // Use indices, not vector references invalidated by recursive emplace_back.
        nodes[index].left = left;
        nodes[index].right = right;
    }
    return index;
}

bool BVH::Hit(const Ray &ray, double lo, double hi, HitRecord &hit, QueryStats *stats) const {
    return !nodes.empty() && HitNode(0, ray, lo, hi, hit, stats);
}

bool BVH::HitNode(
    int index, const Ray &ray, double lo, double hi, HitRecord &hit, QueryStats *stats) const {
    const BVHNode &node = nodes[index];
    if (stats) ++stats->aabb_tests;
    if (!node.bounds.Hit(ray, lo, hi)) return false;
    bool found = false;
    HitRecord best, candidate;
    if (node.count) {
        for (std::size_t i = node.start; i < node.start + node.count; ++i) {
            if (IntersectPrimitive(*primitives[indices[i]], ray, lo, hi, candidate, stats)
                && (!found || PreferHit(candidate, best))) {
                best = candidate;
                hi = best.t;
                found = true;
            }
        }
    } else {
        if (HitNode(node.left, ray, lo, hi, best, stats)) {
            hi = best.t;
            found = true;
        }
        if (HitNode(node.right, ray, lo, hi, candidate, stats)
            && (!found || PreferHit(candidate, best))) {
            best = candidate;
            found = true;
        }
    }
    if (found) hit = best;
    return found;
}
} // namespace a3
