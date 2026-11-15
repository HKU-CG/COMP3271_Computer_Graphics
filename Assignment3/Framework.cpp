#include "Renderer.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <format>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace a3 {
namespace {
void Add(Scene &s, std::unique_ptr<Hittable> p) {
    p->id = static_cast<std::uint32_t>(s.primitives.size());
    s.primitives.push_back(std::move(p));
}

void Quad(Scene &s, Vec3 a, Vec3 b, Vec3 c, Vec3 d, Vec3 normal, Material m) {
    if (Dot(Cross(b - a, c - a), normal) < 0) std::swap(b, d);
    Add(s, std::make_unique<Triangle>(a, b, c, m));
    Add(s, std::make_unique<Triangle>(a, c, d, m));
}

void Box(Scene &s, Vec3 lo, Vec3 hi, Material m) {
    double x = lo.x, y = lo.y, z = lo.z, X = hi.x, Y = hi.y, Z = hi.z;
    Quad(s, {x, y, z}, {x, y, Z}, {x, Y, Z}, {x, Y, z}, {-1, 0, 0}, m);
    Quad(s, {X, y, z}, {X, Y, z}, {X, Y, Z}, {X, y, Z}, {1, 0, 0}, m);
    Quad(s, {x, y, z}, {X, y, z}, {X, y, Z}, {x, y, Z}, {0, -1, 0}, m);
    Quad(s, {x, Y, z}, {x, Y, Z}, {X, Y, Z}, {X, Y, z}, {0, 1, 0}, m);
    Quad(s, {x, y, z}, {x, Y, z}, {X, Y, z}, {X, y, z}, {0, 0, -1}, m);
    Quad(s, {x, y, Z}, {X, y, Z}, {X, Y, Z}, {x, Y, Z}, {0, 0, 1}, m);
}
} // namespace

Scene MakeScene(const std::string &name) {
    if (name != "cornell" && name != "cornell_stress" && name != "cornell_mirror")
        throw std::invalid_argument(std::format("Unknown scene: {}", name));
    Scene s;
    Material white{
        MaterialKind::Diffuse, {0.73, 0.73, 0.73},
         {}
    };
    Material red{
        MaterialKind::Diffuse, {0.65, 0.05, 0.05},
         {}
    };
    Material green{
        MaterialKind::Diffuse, {0.12, 0.45, 0.15},
         {}
    };
    Material light{
        MaterialKind::Emissive, {},
         {12, 12, 12}
    };
    Quad(s, {-1, 0, 0}, {-1, 0, 2}, {-1, 2, 2}, {-1, 2, 0}, {1, 0, 0}, red);
    Quad(s, {1, 0, 0}, {1, 2, 0}, {1, 2, 2}, {1, 0, 2}, {-1, 0, 0}, green);
    Quad(s, {-1, 0, 0}, {1, 0, 0}, {1, 0, 2}, {-1, 0, 2}, {0, 1, 0}, white);
    Quad(s, {-1, 2, 0}, {-1, 2, 2}, {1, 2, 2}, {1, 2, 0}, {0, -1, 0}, white);
    Quad(s, {-1, 0, 2}, {1, 0, 2}, {1, 2, 2}, {-1, 2, 2}, {0, 0, -1}, white);
    Quad(
        s, {-0.5, 1.99, 0.6}, {0.5, 1.99, 0.6}, {0.5, 1.99, 1.4}, {-0.5, 1.99, 1.4}, {0, -1, 0},
        light);
    if (name == "cornell_stress") {
        for (int k = 0; k < 16; ++k)
            for (int j = 0; j < 16; ++j)
                for (int i = 0; i < 16; ++i)
                    Add(s, std::make_unique<Sphere>(
                               Vec3{-0.84 + 0.112 * i, 0.08 + 0.112 * j, 0.16 + 0.112 * k}, 0.035,
                               white));
    } else {
        Box(s, {-0.75, 0, 0.45}, {-0.15, 0.6, 1.05}, white);
        Material sphere = name == "cornell_mirror"
            ? Material{MaterialKind::Mirror,{0.95,0.95,0.95},{}} : white;
        Add(s, std::make_unique<Sphere>(Vec3{0.4, 0.4, 1.35}, 0.4, sphere));
    }
    return s;
}

World::World(const Scene &s, bool accelerated_) :
    scene(s), accelerated(accelerated_), bvh(s.primitives) {
    if (accelerated) {
        auto start = std::chrono::steady_clock::now();
        bvh.Build();
        build_ms =
            std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start)
                .count();
    }
}

bool World::Hit(const Ray &ray, double lo, double hi, HitRecord &hit, QueryStats *stats) const {
    if (accelerated) return bvh.Hit(ray, lo, hi, hit, stats);
    bool found = false;
    HitRecord best, candidate;
    for (const auto &p : scene.primitives) {
        if (IntersectPrimitive(*p, ray, lo, hi, candidate, stats)
            && (!found || PreferHit(candidate, best))) {
            best = candidate;
            hi = best.t;
            found = true;
        }
    }
    if (found) hit = best;
    return found;
}

Camera::Camera(int width, int height) :
    aspect(double(width) / height), half_height(std::tan(20 * pi / 180)) {
    forward = Normalize(Vec3{0, 1, 1} - origin);
    right = Normalize(Cross(forward, {0, 1, 0}));
    up = Cross(right, forward);
}

Ray Camera::GetRay(double u, double v) const {
    return {
        origin, Normalize(
                    forward + right * ((2 * u - 1) * aspect * half_height)
                    + up * ((1 - 2 * v) * half_height))};
}

std::vector<Color> Render(const World &world, const RenderOptions &o) {
    Camera camera(o.width, o.height);
    std::vector<Color> pixels(std::size_t(o.width) * o.height);
    for (int y = 0; y < o.height; ++y) {
        for (int x = 0; x < o.width; ++x) {
            std::size_t pixel = std::size_t(y) * o.width + x;
            Color sum;
            for (int sample = 0; sample < o.spp; ++sample) {
                // Each sample owns a stream; other pixels and traversal cannot affect it.
                RNG rng(
                    Mix(o.seed ^ Mix(pixel + UINT64_C(0x9e3779b97f4a7c15))
                        ^ Mix(std::uint64_t(sample) + UINT64_C(0xd1b54a32d192ed03))));
                double u = (x + rng.Uniform()) / o.width;
                double v = (y + rng.Uniform()) / o.height;
                Ray ray = camera.GetRay(u, v);
                Color value;
                if (o.preview == "radiance")
                    value = TracePath(world, ray, o.depth, rng);
                else {
                    HitRecord h;
                    if (world.Hit(ray, ray_epsilon, infinity, h)) {
                        if (o.preview == "normal")
                            value = (h.normal + Vec3{1, 1, 1}) * 0.5;
                        else {
                            double d = std::clamp(h.t / 8.0, 0.0, 1.0);
                            value = {d, d, d};
                        }
                    }
                }
                if (!Finite(value))
                    throw std::runtime_error("Nonfinite radiance: inspect student implementation");
                sum = sum + value;
            }
            pixels[pixel] = sum / double(o.spp);
        }
        if (y % 32 == 0) spdlog::info("Rendering row {}/{}", y + 1, o.height);
    }
    spdlog::info("Rendering complete.");
    return pixels;
}

int DisplayChannel(double linear) {
    if (!std::isfinite(linear)) throw std::runtime_error("Cannot encode nonfinite radiance");
    return int(256 * std::clamp(std::sqrt(std::max(0.0, linear)), 0.0, 0.999));
}

void WritePPM(
    const std::string &filename, int width, int height, const std::vector<Color> &pixels) {
    if (width <= 0 || height <= 0 || pixels.size() != std::size_t(width) * height)
        throw std::invalid_argument("Invalid image dimensions or pixel count");
    std::filesystem::path path(filename);
    if (!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path);
    if (!out) throw std::runtime_error(std::format("Cannot open output: {}", filename));
    out << std::format("P3\n{} {}\n255\n", width, height);
    for (Color c : pixels)
        out << std::format(
            "{} {} {}\n", DisplayChannel(c.x), DisplayChannel(c.y), DisplayChannel(c.z));
    if (!out) throw std::runtime_error(std::format("Failed writing output: {}", filename));
}
} // namespace a3
