#include "Renderer.hpp"
#include <argparse/argparse.hpp>
#include <chrono>
#include <format>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string_view>

namespace {
int Bounded(
    const argparse::ArgumentParser &args, std::string_view name, int fallback, int maximum,
    int minimum = 1) {
    int value = args.present<int>(name).value_or(fallback);
    if (value < minimum || value > maximum)
        throw std::invalid_argument(
            std::format("{} out of range ({}..{})", name, minimum, maximum));
    return value;
}
} // namespace

int main(int argc, char **argv) {
    // All console messages go through spdlog; a message-only pattern also keeps
    // the benchmark's CSV rows copyable. Image data is written only to its file.
    spdlog::set_default_logger(spdlog::stderr_color_mt("assignment3"));
    spdlog::set_pattern("%v");
    argparse::ArgumentParser args("assignment3", "1.0", argparse::default_arguments::none);
    args.add_description("COMP3271 Assignment 3: CPU path tracing and BVH");
    args.add_argument("-h", "--help").flag().help("Show this help message");
    args.add_argument("--scene")
        .choices("cornell", "cornell_stress", "cornell_mirror")
        .help(
            "Scene (default cornell); benchmark defaults to cornell_stress unless explicitly "
            "selected");
    args.add_argument("--accel")
        .choices("brute", "bvh")
        .help("Intersection accelerator: brute = no acceleration, bvh = BVH (default brute)");
    args.add_argument("--width").scan<'d', int>().help("Image width (1..16384, default 256)");
    args.add_argument("--height").scan<'d', int>().help("Image height (1..16384, default 256)");
    args.add_argument("--spp").scan<'d', int>().help("Samples per pixel (1..1000000, default 128)");
    args.add_argument("--depth").scan<'d', int>().help(
        "Maximum scattering events (0..1024, default 8)");
    args.add_argument("--seed").scan<'u', std::uint64_t>().help(
        "Nonnegative 64-bit random seed (default 3271)");
    args.add_argument("--output")
        .help("PPM path relative to working directory (default outputs/cornell.ppm)");
    args.add_argument("--preview")
        .choices("radiance", "normal", "depth")
        .help("Radiance or geometry debug view (default radiance)");
    auto &mode = args.add_mutually_exclusive_group();
    mode.add_argument("--check").flag().help("Run public correctness checks");
    mode.add_argument("--benchmark").flag().help("Compare both accelerators over three runs");
    args.add_argument("--rays").scan<'d', int>().help(
        "Benchmark ray count (1..10000000, default 10000)");
    try {
        args.parse_args(argc, argv);
        if (args.get<bool>("--help")) {
            spdlog::info("{}", args.help().str());
            return 0;
        }
        a3::RenderOptions o;
        o.width = Bounded(args, "--width", o.width, 16384);
        o.height = Bounded(args, "--height", o.height, 16384);
        o.spp = Bounded(args, "--spp", o.spp, 1000000);
        o.depth = Bounded(args, "--depth", o.depth, 1024, 0);
        o.seed = args.present<std::uint64_t>("--seed").value_or(o.seed);
        o.scene = args.present<std::string>("--scene").value_or(o.scene);
        if (auto accel = args.present<std::string>("--accel")) o.accelerated = *accel == "bvh";
        o.preview = args.present<std::string>("--preview").value_or(o.preview);
        o.output = args.present<std::string>("--output").value_or(o.output);
        if (o.output.empty()) throw std::invalid_argument("Empty output path");
        std::size_t rays = static_cast<std::size_t>(Bounded(args, "--rays", 10000, 10000000));
        if (args.get<bool>("--check")) return a3::RunChecks();
        if (args.get<bool>("--benchmark"))
            return a3::RunBenchmark(
                args.is_used("--scene") ? o.scene : "cornell_stress", rays, o.seed);
        auto scene = a3::MakeScene(o.scene);
        a3::World world(scene, o.accelerated);
        auto start = std::chrono::steady_clock::now();
        auto pixels = a3::Render(world, o);
        double seconds =
            std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        a3::WritePPM(o.output, o.width, o.height, pixels);
        spdlog::info(
            "Wrote {} ({}x{}, {} spp, depth {}, seed {}, {})", o.output, o.width, o.height, o.spp,
            o.depth, o.seed, o.accelerated ? "BVH" : "brute force");
        spdlog::info("Build: {:.6f} ms; render: {:.6f} s", world.build_ms, seconds);
        return 0;
    } catch (const std::exception &e) {
        spdlog::error("Error: {}\nUse --help for usage.", e.what());
        return 1;
    }
}
