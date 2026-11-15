#pragma once

// Provided framework: double-precision math, deterministic RNG, scene and query
// interfaces. Assessed implementations are marked in the four corresponding .cpp files.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace a3 {
constexpr double pi = 3.14159265358979323846;
constexpr double ray_epsilon = 1e-4;
constexpr double infinity = std::numeric_limits<double>::infinity();

struct Vec3 {
    double x = 0, y = 0, z = 0;

    double operator[](int i) const { return i == 0 ? x : (i == 1 ? y : z); }

    double &operator[](int i) { return i == 0 ? x : (i == 1 ? y : z); }
};

using Color = Vec3;

inline Vec3 operator+(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }

inline Vec3 operator-(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }

inline Vec3 operator-(Vec3 a) { return {-a.x, -a.y, -a.z}; }

inline Vec3 operator*(Vec3 a, double s) { return {a.x * s, a.y * s, a.z * s}; }

inline Vec3 operator*(double s, Vec3 a) { return a * s; }

inline Vec3 operator/(Vec3 a, double s) { return a * (1 / s); }

inline Vec3 operator*(Vec3 a, Vec3 b) { return {a.x * b.x, a.y * b.y, a.z * b.z}; }

inline double Dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

inline Vec3 Cross(Vec3 a, Vec3 b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

inline double Length(Vec3 v) { return std::sqrt(Dot(v, v)); }

inline Vec3 Normalize(Vec3 v) {
    double n = Length(v);
    return n > 0 ? v / n : Vec3{};
}

inline Vec3 Min(Vec3 a, Vec3 b) {
    return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)};
}

inline Vec3 Max(Vec3 a, Vec3 b) {
    return {std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)};
}

inline bool Finite(Vec3 v) {
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}

// SplitMix64; Uniform never returns 1. Traversal must never consume this stream.
inline std::uint64_t Mix(std::uint64_t x) {
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    return x ^ (x >> 31);
}

struct RNG {
    std::uint64_t state;

    explicit RNG(std::uint64_t seed) : state(seed) { }

    double Uniform() {
        state += UINT64_C(0x9e3779b97f4a7c15);
        return static_cast<double>(Mix(state) >> 11) * 0x1.0p-53;
    }
};

struct Ray {
    Vec3 origin, direction;

    Vec3 At(double t) const { return origin + direction * t; }
};
enum class MaterialKind { Diffuse, Emissive, Mirror };

struct Material {
    MaterialKind kind = MaterialKind::Diffuse;
    Color reflectance{}, emission{};
};

struct HitRecord {
    double t = infinity;
    Vec3 position{}, normal{};
    bool front_face = false;
    const Material *material = nullptr;
    std::uint32_t primitive_id = std::numeric_limits<std::uint32_t>::max();
};

inline bool PreferHit(const HitRecord &a, const HitRecord &b) {
    return a.t < b.t || (a.t == b.t && a.primitive_id < b.primitive_id);
}

struct QueryStats {
    std::uint64_t primitive_tests = 0, aabb_tests = 0;
};

struct AABB {
    Vec3 minimum{infinity, infinity, infinity}, maximum{-infinity, -infinity, -infinity};

    void Expand(const AABB &b) {
        minimum = Min(minimum, b.minimum);
        maximum = Max(maximum, b.maximum);
    }

    Vec3 Centroid() const { return (minimum + maximum) * 0.5; }

    bool Hit(const Ray &ray, double t_min, double t_max) const;
};

class Hittable {
  public:
    Material material;
    std::uint32_t id = 0;

    explicit Hittable(Material m) : material(m) { }

    virtual ~Hittable() = default;
    virtual bool Hit(const Ray &, double t_min, double t_max, HitRecord &) const = 0;
    virtual AABB Bounds() const = 0;

  protected:
    void SetHit(const Ray &ray, double t, Vec3 outward, HitRecord &hit) const {
        hit.t = t;
        hit.position = ray.At(t);
        hit.front_face = Dot(ray.direction, outward) < 0;
        hit.normal = hit.front_face ? outward : -outward;
        hit.material = &material;
        hit.primitive_id = id;
    }
};

class Sphere final : public Hittable {
  public:
    Vec3 center;
    double radius;

    Sphere(Vec3 c, double r, Material m) : Hittable(m), center(c), radius(r) { }

    bool Hit(const Ray &, double, double, HitRecord &) const override;

    AABB Bounds() const override {
        Vec3 r{radius, radius, radius};
        return {center - r, center + r};
    }
};

class Triangle final : public Hittable {
  public:
    Vec3 a, b, c;

    Triangle(Vec3 a_, Vec3 b_, Vec3 c_, Material m) : Hittable(m), a(a_), b(b_), c(c_) { }

    bool Hit(const Ray &, double, double, HitRecord &) const override;

    AABB Bounds() const override { return {Min(a, Min(b, c)), Max(a, Max(b, c))}; }
};

using PrimitiveList = std::vector<std::unique_ptr<Hittable>>;

// Both accelerators count exactly one test per primitive call.
inline bool IntersectPrimitive(
    const Hittable &p, const Ray &r, double lo, double hi, HitRecord &h, QueryStats *stats) {
    if (stats) ++stats->primitive_tests;
    return p.Hit(r, lo, hi, h);
}

struct BVHNode {
    AABB bounds;
    int left = -1, right = -1;
    std::size_t start = 0, count = 0;
};

class BVH {
  public:
    explicit BVH(const PrimitiveList &p) : primitives(p) { }

    void Build();
    bool Hit(const Ray &, double, double, HitRecord &, QueryStats * = nullptr) const;

    const std::vector<BVHNode> &Nodes() const { return nodes; }

    const std::vector<std::size_t> &Indices() const { return indices; }

  private:
    const PrimitiveList &primitives;
    std::vector<BVHNode> nodes;
    std::vector<std::size_t> indices;
    int BuildRange(std::size_t begin, std::size_t end);
    bool HitNode(int, const Ray &, double, double, HitRecord &, QueryStats *) const;
};

struct Scene {
    PrimitiveList primitives;
};

Scene MakeScene(const std::string &name);

class World {
  public:
    World(const Scene &s, bool accelerated);
    bool Hit(const Ray &, double, double, HitRecord &, QueryStats * = nullptr) const;

    const BVH &Tree() const { return bvh; }

    double build_ms = 0;

  private:
    const Scene &scene;
    bool accelerated;
    BVH bvh;
};

struct Camera {
    Vec3 origin{0, 1, -3.2}, forward{}, right{}, up{};
    double aspect, half_height;
    Camera(int width, int height);
    // u goes left to right; v goes top to bottom.
    Ray GetRay(double u, double v) const;
};

Vec3 SampleHemisphere(Vec3 normal, RNG &rng);
bool ScatterMirror(const Ray &, const HitRecord &, Vec3 &direction, Color &weight);
Color TracePath(const World &, Ray, int max_depth, RNG &);

struct RenderOptions {
    int width = 256, height = 256, spp = 128, depth = 8;
    std::uint64_t seed = 3271;
    bool accelerated = false;
    std::string scene = "cornell", output = "outputs/cornell.ppm", preview = "radiance";
};

std::vector<Color> Render(const World &, const RenderOptions &);
int DisplayChannel(double linear);
void WritePPM(const std::string &, int width, int height, const std::vector<Color> &);
int RunChecks();
int RunBenchmark(const std::string &, std::size_t ray_count, std::uint64_t seed);
} // namespace a3
