// Provided public diagnostics. These checks do not replace any student algorithm.
#include "Renderer.hpp"
#include <chrono>
#include <functional>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace a3 {
namespace {
void Require(bool ok, const std::string &message) {
    if (!ok) throw std::runtime_error(message);
}

bool Near(double a, double b, double eps = 1e-5) {
    return std::isfinite(a) && std::isfinite(b)
           && std::abs(a - b) <= eps * std::max(1.0, std::abs(b));
}

bool Near(Vec3 a, Vec3 b, double eps = 1e-5) {
    return Near(a.x, b.x, eps) && Near(a.y, b.y, eps) && Near(a.z, b.z, eps);
}

Material Matte(Color c = {0.5, 0.5, 0.5}) { return {MaterialKind::Diffuse, c, {}}; }

void Add(Scene &s, std::unique_ptr<Hittable> p) {
    p->id = static_cast<std::uint32_t>(s.primitives.size());
    s.primitives.push_back(std::move(p));
}

void SameHit(bool a, const HitRecord &ha, bool b, const HitRecord &hb) {
    Require(a == b, "Hit/miss disagreement");
    if (!a) return;
    Require(
        Near(ha.t, hb.t) && Near(ha.position, hb.position) && Near(ha.normal, hb.normal),
        "Distance, position, or normal disagreement");
    Require(
        ha.primitive_id == hb.primitive_id && ha.front_face == hb.front_face
            && ha.material == hb.material,
        "Primitive/material/sidedness disagreement");
}

void CheckPrimitives() {
    Sphere s({0, 0, 0}, 1, Matte());
    s.id = 17;
    HitRecord h;
    Require(
        s.Hit(
            {
                {0, 0, -3},
                {0, 0, 2 }
    },
            0, infinity, h)
            && Near(h.t, 1) && h.front_face && Near(h.normal, {0, 0, -1}) && h.primitive_id == 17
            && h.material == &s.material,
        "Sphere: nonunit direction or record");
    Require(
        s.Hit(
            {
                {0, 0, 0},
                {1, 0, 0}
    },
            0, infinity, h)
            && Near(h.t, 1) && !h.front_face && Near(h.normal, {-1, 0, 0}),
        "Sphere: inside ray");
    Require(
        s.Hit(
            {
                {1, 0, -2},
                {0, 0, 1 }
    },
            0, infinity, h)
            && Near(h.t, 2),
        "Sphere: tangent");
    Require(
        s.Hit(
            {
                {0, 0, -3},
                {0, 0, 1 }
    },
            2.5, 4, h)
            && Near(h.t, 4),
        "Sphere: second root / endpoint");
    h.t = 123;
    Require(
        !s.Hit(
            {
                {0, 0, -3},
                {0, 0, 1 }
    },
            2.5, 3.5, h)
            && h.t == 123,
        "Sphere: clipping / miss record");
    Require(
        !s.Hit(
            {
                {0, 0, -3},
                {0, 0, 0 }
    },
            0, infinity, h),
        "Sphere: zero ray");
    Triangle t({0, 0, 0}, {1, 0, 0}, {0, 1, 0}, Matte());
    Require(
        t.Hit(
            {
                {0.2, 0.2, 1 },
                {0,   0,   -2}
    },
            0, 1, h)
            && Near(h.t, 0.5) && h.front_face && Near(h.normal, {0, 0, 1}),
        "Triangle: front/nonunit");
    Require(
        t.Hit(
            {
                {0.2, 0.2, -1},
                {0,   0,   1 }
    },
            0, 1, h)
            && !h.front_face,
        "Triangle: backface");
    Require(
        t.Hit(
            {
                {0.5, 0.5, 1 },
                {0,   0,   -1}
    },
            0, 1, h),
        "Triangle: edge");
    Require(
        t.Hit(
            {
                {0, 0, 1 },
                {0, 0, -1}
    },
            0, 1, h),
        "Triangle: vertex");
    h.t = 123;
    Require(
        !t.Hit(
            {
                {0.8, 0.8, 1 },
                {0,   0,   -1}
    },
            0, infinity, h)
            && h.t == 123,
        "Triangle: outside / record");
    Require(
        !t.Hit(
            {
                {0.2, 0.2, 1},
                {1,   0,   0}
    },
            0, infinity, h),
        "Triangle: parallel");
    Require(
        !t.Hit(
            {
                {0.2, 0.2, 1 },
                {0,   0,   -1}
    },
            0, 0.5, h),
        "Triangle: t_max");
    Triangle degenerate({0, 0, 0}, {1, 0, 0}, {2, 0, 0}, Matte());
    Require(
        !degenerate.Hit(
            {
                {0, 0, 1 },
                {0, 0, -1}
    },
            0, infinity, h),
        "Triangle: degenerate");
}

void CheckSampling() {
    std::array<Vec3, 8> normals{
        {{1, 0, 0},
         {-1, 0, 0},
         {0, 1, 0},
         {0, -1, 0},
         {0, 0, 1},
         {0, 0, -1},
         Normalize({1, 2, 3}),
         Normalize({1, 1e-10, 0})}
    };
    RNG rng(3271);
    constexpr int n = 50000;
    for (Vec3 normal : normals) {
        Vec3 mean;
        double cosine = 0, square = 0;
        for (int i = 0; i < n; ++i) {
            Vec3 d = SampleHemisphere(normal, rng);
            double c = Dot(d, normal);
            Require(
                Finite(d) && Near(Length(d), 1, 1e-9) && c >= -1e-12,
                "Sampler: unit hemisphere invariant");
            mean = mean + d;
            cosine += c;
            square += c * c;
        }
        mean = mean / double(n);
        cosine /= n;
        square /= n;
        Require(
            std::abs(cosine - 2.0 / 3) < 0.012 && std::abs(square - 0.5) < 0.012
                && Length(mean - normal * cosine) < 0.015,
            "Sampler: cosine distribution moments");
    }
}

void CheckBounds() {
    AABB b{
        {-1, -1, -1},
        {1,  1,  1 }
    };
    Require(
        b.Hit(
            {
                {0, 0, -3},
                {0, 0, 1 }
    },
            0, 10),
        "AABB: positive / parallel inside");
    Require(
        b.Hit(
            {
                {0, 0, 3 },
                {0, 0, -1}
    },
            0, 10),
        "AABB: negative");
    Require(
        b.Hit(
            {
                {0, 0, 0},
                {1, 0, 0}
    },
            0, 10),
        "AABB: starts inside");
    Require(
        !b.Hit(
            {
                {2, 0, -3},
                {0, 0, 1 }
    },
            0, 10),
        "AABB: parallel outside");
    Require(
        b.Hit(
            {
                {1, 0, -3},
                {0, 0, 1 }
    },
            2, 2),
        "AABB: boundary and touching interval");
    Require(
        !b.Hit(
            {
                {0, 0, -3},
                {0, 0, 1 }
    },
            0, 1),
        "AABB: clipped interval");
    Require(
        !b.Hit(
            {
                {0, 0, 0},
                {1, 0, 0}
    },
            2, 1),
        "AABB: reversed interval");
    AABB flat{
        {0, 0, 0},
        {1, 1, 0}
    };
    Require(
        flat.Hit(
            {
                {0.25, 0.25, 1 },
                {0,    0,    -1}
    },
            1, 1),
        "AABB: flat triangle bounds");
    Require(
        !flat.Hit(
            {
                {0.25, 0.25, 1},
                {1,    0,    0}
    },
            0, 10),
        "AABB: parallel to flat box");
    Require(
        !AABB{
    }
             .Hit({{0, 0, 0}, {1, 0, 0}}, 0, 10),
        "AABB: empty bounds");
}

void ValidateTree(const Scene &scene, const BVH &tree) {
    const auto &nodes = tree.Nodes();
    const auto &indices = tree.Indices();
    if (scene.primitives.empty()) {
        Require(nodes.empty(), "Empty BVH not empty");
        return;
    }
    Require(
        !nodes.empty() && indices.size() == scene.primitives.size(),
        "Missing nodes / index coverage");
    std::vector<int> seen(scene.primitives.size()), visited(nodes.size());
    auto encloses = [](const AABB &a, const AABB &b) {
        for (int k = 0; k < 3; ++k)
            if (a.minimum[k] > b.minimum[k] + 1e-12 || a.maximum[k] < b.maximum[k] - 1e-12)
                return false;
        return true;
    };
    std::function<void(int)> visit = [&](int i) {
        Require(i >= 0 && std::size_t(i) < nodes.size(), "Invalid child index");
        Require(++visited[i] == 1, "Cycle / duplicate node");
        const auto &n = nodes[i];
        Require(Finite(n.bounds.minimum) && Finite(n.bounds.maximum), "Nonfinite node bounds");
        if (n.count) {
            Require(n.count <= 4 && n.start + n.count <= indices.size(), "Invalid leaf range");
            Require(n.left == -1 && n.right == -1, "Leaf has children");
            for (std::size_t j = n.start; j < n.start + n.count; ++j) {
                auto p = indices[j];
                Require(p < seen.size(), "Invalid primitive index");
                ++seen[p];
                Require(
                    encloses(n.bounds, scene.primitives[p]->Bounds()),
                    "Leaf bounds omit primitive");
            }
        } else {
            visit(n.left);
            visit(n.right);
            Require(
                encloses(n.bounds, nodes[n.left].bounds)
                    && encloses(n.bounds, nodes[n.right].bounds),
                "Parent bounds omit child");
        }
    };
    visit(0);
    for (int count : seen) Require(count == 1, "Primitive not present exactly once");
    for (int count : visited) Require(count == 1, "Unreachable node");
}

void CheckTrees() {
    for (int count : {0, 1, 5, 17}) {
        Scene s;
        for (int i = 0; i < count; ++i) Add(s, std::make_unique<Sphere>(Vec3{}, 1, Matte()));
        World b(s, false), v(s, true);
        ValidateTree(s, v.Tree());
        HitRecord hb, hv;
        bool fb = b.Hit(
            {
                {0, 0, -3},
                {0, 0, 1 }
        },
            0, infinity, hb);
        bool fv = v.Hit(
            {
                {0, 0, -3},
                {0, 0, 1 }
        },
            0, infinity, hv);
        SameHit(fb, hb, fv, hv);
        if (count)
            Require(hv.primitive_id == 0, "Equal-distance primitive ID tie");
        else {
            hv.t = 123;
            Require(
                !v.Hit(
                    {
                        {0, 0, 0},
                        {1, 0, 0}
            },
                    0, infinity, hv)
                    && hv.t == 123,
                "Empty BVH miss record");
        }
    }
}

void CheckEquivalence() {
    RNG rng(3271);
    Camera camera(256, 256);
    for (const std::string name : {"cornell", "cornell_stress", "cornell_mirror"}) {
        auto s = MakeScene(name);
        World brute(s, false), bvh(s, true);
        ValidateTree(s, bvh.Tree());
        for (int i = 0; i < 3000; ++i) {
            Ray ray;
            if (i < 1000) {
                double u = rng.Uniform(), v = rng.Uniform();
                ray = camera.GetRay(u, v);
            } else {
                Vec3 origin{rng.Uniform() * 4 - 2, rng.Uniform() * 4 - 1, rng.Uniform() * 4 - 1};
                Vec3 d{rng.Uniform() * 2 - 1, rng.Uniform() * 2 - 1, rng.Uniform() * 2 - 1};
                ray = {origin, d}; // Includes inside rays and nonunit directions.
            }
            double hi = i % 3 == 0 ? 0.5 : infinity;
            HitRecord a, b;
            bool fa = brute.Hit(ray, ray_epsilon, hi, a), fb = bvh.Hit(ray, ray_epsilon, hi, b);
            SameHit(fa, a, fb, b);
        }
    }
}

Scene TransportFixture(bool mirror) {
    Scene s;
    Material floor = mirror ? Material{MaterialKind::Mirror,{0.2,0.4,0.6},{}} : Matte({0.2,0.4,0.6});
    Material light{
        MaterialKind::Emissive, {},
         {2, 3, 4}
    };
    constexpr double q = 1e6;
    Add(s, std::make_unique<Triangle>(Vec3{-q, 0, -q}, Vec3{-q, 0, q}, Vec3{q, 0, q}, floor));
    Add(s, std::make_unique<Triangle>(Vec3{-q, 0, -q}, Vec3{q, 0, q}, Vec3{q, 0, -q}, floor));
    Add(s, std::make_unique<Triangle>(Vec3{-q, 1, -q}, Vec3{q, 1, -q}, Vec3{q, 1, q}, light));
    Add(s, std::make_unique<Triangle>(Vec3{-q, 1, -q}, Vec3{q, 1, q}, Vec3{-q, 1, q}, light));
    return s;
}

void CheckTransport(bool mirror) {
    auto s = TransportFixture(mirror);
    World world(s, false);
    auto trace = [&](Ray r, int depth) {
        RNG rng(3271);
        return TracePath(world, r, depth, rng);
    };
    Require(
        Near(
            trace(
                {
                    {0, 0.5, 0},
                    {0, 1,   0}
    },
                0),
            {2, 3, 4}),
        "Transport: camera-visible emission at depth 0");
    Require(
        Near(
            trace(
                {
                    {0, 2,  0},
                    {0, -1, 0}
    },
                8),
            {}),
        "Transport: backface light emission");
    Require(
        Near(
            trace(
                {
                    {0, 0.5, 0},
                    {0, -1,  0}
    },
                0),
            {}),
        "Transport: depth 0 must not scatter");
    Require(
        Near(
            trace(
                {
                    {0, 0.5, 0},
                    {0, -1,  0}
    },
                1),
            {0.4, 1.2, 2.4}),
        "Transport: terminal emission / reflectance weighting (radiance must remain unclamped)");
    Require(
        Near(
            trace(
                {
                    {0, 0.5, 0},
                    {0, -1,  0}
    },
                8),
            {0.4, 1.2, 2.4}),
        "Transport: spurious depth attenuation");
    Require(
        Near(
            trace(
                {
                    {0, 2, 0},
                    {0, 1, 0}
    },
                8),
            {}),
        "Transport: black background");
    if (mirror) {
        Sphere sphere(
            {
        },
            1, Material{MaterialKind::Mirror, {0.8, 0.6, 0.4}, {}});
        Ray ray{
            {0,   0, -3},
            Normalize({0.2, 0, 1 }
            )
        };
        HitRecord hit;
        Require(sphere.Hit(ray, 0, infinity, hit), "Mirror fixture did not hit sphere");
        Vec3 direction;
        Color weight;
        Require(ScatterMirror(ray, hit, direction, weight), "Mirror scatter unavailable");
        Require(
            Near(Length(direction), 1) && Dot(direction, hit.normal) > 0
                && Near(Dot(direction, hit.normal), -Dot(ray.direction, hit.normal))
                && Near(
                    direction + hit.normal * Dot(ray.direction, hit.normal),
                    ray.direction - hit.normal * Dot(ray.direction, hit.normal))
                && Near(weight, {0.8, 0.6, 0.4}),
            "Mirror reflection angle / throughput");
    }
}

void CheckRender() {
    auto s = MakeScene("cornell");
    World brute(s, false), bvh(s, true);
    RenderOptions o;
    o.width = 16;
    o.height = 16;
    o.spp = 8;
    auto a = Render(brute, o), b = Render(bvh, o);
    double energy = 0;
    for (std::size_t i = 0; i < a.size(); ++i) {
        Require(Near(a[i], b[i], 1e-10), "Brute/BVH radiance disagreement");
        energy += a[i].x + a[i].y + a[i].z;
    }
    Require(energy > 0, "Render is completely black");
    Require(
        DisplayChannel(0.25) == 128 && DisplayChannel(-1) == 0 && DisplayChannel(10) == 255,
        "PPM gamma/clamp conversion");
    Camera camera(16, 16);
    Require(
        camera.GetRay(0.5, 0).direction.y > camera.GetRay(0.5, 1).direction.y,
        "Top-row orientation");
}
} // namespace

int RunChecks() {
    int failures = 0;
    auto group = [&](const char *name, const std::function<void()> &test, bool bonus = false) {
        try {
            test();
            spdlog::info("[PASS] {}", name);
        } catch (const std::exception &e) {
            if (!bonus) ++failures;
            spdlog::log(
                bonus ? spdlog::level::warn : spdlog::level::err, "{} {}: {}",
                bonus ? "[BONUS incomplete]" : "[FAIL]", name, e.what());
        }
    };
    group("Primitive intersections", CheckPrimitives);
    group("Cosine sampling (50,000 samples per normal; documented tolerances)", CheckSampling);
    group("AABB slabs", CheckBounds);
    group("BVH structure and equal-centroid cases", CheckTrees);
    group("Brute/BVH primary and secondary ray equivalence", CheckEquivalence);
    group("Diffuse transport and terminal emission", [] {
        CheckTransport(false);
    });
    group("Render equivalence and display conversion", CheckRender);
    group(
        "Perfect mirror (optional)",
        [] {
            CheckTransport(true);
        },
        true);
    spdlog::info("{} required check group(s) failed.", failures);
    return failures ? 1 : 0;
}

int RunBenchmark(const std::string &name, std::size_t count, std::uint64_t seed) {
    auto scene = MakeScene(name);
    World brute(scene, false), bvh(scene, true);
    RNG rng(seed);
    Camera camera(256, 256);
    std::vector<Ray> rays;
    rays.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        double u = rng.Uniform(), v = rng.Uniform();
        rays.push_back(camera.GetRay(u, v));
    }
    std::array<std::vector<HitRecord>, 2> hits;
    std::array<QueryStats, 2> stats{};
    std::array<double, 2> median{};
    std::array<const World *, 2> worlds{
        {&brute, &bvh}
    };
    spdlog::info(
        "Scene: {}; primitives: {}; rays: {}; seed: {}; threads: 1; runs: 3", name,
        scene.primitives.size(), count, seed);
    spdlog::info("mode,primitive_tests,aabb_tests,median_query_ms,build_ms,checksum");
    for (int mode = 0; mode < 2; ++mode) {
        hits[mode].resize(count);
        std::array<double, 3> times{};
        for (int run = 0; run < 3; ++run) {
            QueryStats current;
            auto start = std::chrono::steady_clock::now();
            for (std::size_t i = 0; i < count; ++i) {
                hits[mode][i] = HitRecord{};
                worlds[mode]->Hit(rays[i], ray_epsilon, infinity, hits[mode][i], &current);
            }
            times[run] =
                std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start)
                    .count();
            if (run == 0)
                stats[mode] = current;
            else
                Require(
                    current.primitive_tests == stats[mode].primitive_tests
                        && current.aabb_tests == stats[mode].aabb_tests,
                    "Nondeterministic benchmark counters");
        }
        std::sort(times.begin(), times.end());
        median[mode] = times[1];
        double checksum = 0;
        for (const auto &h : hits[mode])
            if (h.material) checksum += h.t + double(h.primitive_id) * 0.001;
        spdlog::info(
            "{},{},{},{:.6f},{:.6f},{:.6f}", mode ? "bvh" : "brute", stats[mode].primitive_tests,
            stats[mode].aabb_tests, median[mode], worlds[mode]->build_ms, checksum);
    }
    for (std::size_t i = 0; i < count; ++i)
        SameHit(
            hits[0][i].material != nullptr, hits[0][i], hits[1][i].material != nullptr, hits[1][i]);
    Require(
        stats[0].primitive_tests == std::uint64_t(scene.primitives.size()) * count,
        "Brute force must test every primitive per query");
    if (stats[1].primitive_tests)
        spdlog::info(
            "Primitive-test reduction: {:.6f}x",
            double(stats[0].primitive_tests) / stats[1].primitive_tests);
    if (median[1] > 0) spdlog::info("Query time speedup: {:.6f}x", median[0] / median[1]);
    spdlog::info("All benchmark ray results agree.");
    return 0;
}
} // namespace a3
