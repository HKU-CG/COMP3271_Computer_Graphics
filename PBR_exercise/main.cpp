#include <iostream>
#include <vector>
#include <thread>

#include "as3.h"
#include "Common.h"
#include "Scene.h"
#include "Camera.h"
#include "Material.h"
#include "Hittable.h"
#include "Utils/lodepng.h"

// TODO: feel free to modify the following parameters
const int kMaxTraceDepth = 5;
const int spp = 16;
const int NUM_THREAD = 4;
const std::string scene_file = "scene/cornell_box.toml";
const std::string output_path = "outputs/output.png";

Color TraceRay(const Ray &ray, const Hittable &hittable_collection,
               int trace_depth);

Color Shade(const Hittable &hittable_collection,
            const HitRecord &hit_record,
            int trace_depth);

void tracePixel(int x, int y, int width, int height, int thread_spp, const Camera &camera, const Scene &scene,
                float *image_buffer) {
    Color color(0.f, 0.f, 0.f);
    for (int i = 0; i < thread_spp; ++i) {
        float bias_x = get_random_float();
        float bias_y = get_random_float();
        Ray ray = camera.RayAt(float(x) + bias_x, float(y) + bias_y);
        color += TraceRay(ray, scene.hittable_collection_, 1);
    }
    color /= float(thread_spp);
    int idx = 3 * ((height - y - 1) * width + x);
    image_buffer[idx] += color.r;
    image_buffer[idx + 1] += color.g;
    image_buffer[idx + 2] += color.b;
}

void
thread_func(int thread_i, int n_thread, int total_spp, int width, int height, const Camera &camera, const Scene &scene,
            float *image_buffer) {
    auto now = std::chrono::high_resolution_clock::now();
    float progress = 0.f;

    for (int x = thread_i; x < width; x += n_thread) {
        for (int y = 0; y < height; ++y) {
            if(x < 0 || x >= width || y < 0 || y >= height) continue;
            tracePixel(x, y, width, height, total_spp, camera, scene, image_buffer);
        }
        // Print progress
        progress += 1.f / width * n_thread;
        if (thread_i == 0) {
            std::cout << "Progress: " << progress * 100 << "%" << std::endl;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - now;
    std::cout << "Thread " << thread_i + 1 << " finished in " << elapsed.count() << " seconds." << std::endl;
}

int main() {
    const std::string work_dir(CMAKE_SOURCE_DIR);

    // Construct scene
    Scene scene(work_dir, scene_file);
    const Camera &camera = scene.camera_;
    int width = camera.width_;
    int height = camera.height_;

    std::vector<unsigned char> image(width * height * 4, 0);
    float *image_buffer = new float[width * height * 3]();
    std::cout << "Rendering with " << NUM_THREAD << " threads" << std::endl;
    std::vector<std::thread> thread_pool;
    for (int thread_i = 0; thread_i < NUM_THREAD; ++thread_i) {
        thread_pool.emplace_back(thread_func, thread_i, NUM_THREAD, spp, width, height, std::ref(camera),
                                 std::ref(scene), image_buffer);
    }
    for (std::thread &t: thread_pool) {
        t.join();
    }
    // thread_func(0, 1, spp, width, height, camera, scene, image_buffer);

    // Copy from image_buffer to image
    for (int i = 0; i < width * height; ++i) {
        for (int j = 0; j < 3; ++j) {
            image[4 * i + j] = static_cast<uint8_t>(glm::min(image_buffer[3 * i + j], 1.f - 1e-5f) * 256.f);
        }
        image[4 * i + 3] = 255;
    }


    std::vector<unsigned char> png;
    unsigned error = lodepng::encode(png, image, width, height);
    lodepng::save_file(png, work_dir + output_path);
}

Color Shade(const Hittable &hittable_collection,
            const HitRecord &hit_record,
            int trace_depth) {
    Color color(0.f, 0.f, 0.f);

    const Material &material = hit_record.material;

    // 1. If the object has emission, add its contribution to the color.
    color += material.emission;

    // TODO: Implement the shading function here

    return color;
}

Color TraceRay(const Ray &ray,
               const Hittable &hittable_collection,
               int trace_depth) {
    HitRecord record;
    // Initialize the color as black
    Color color(0.0f, 0.0f, 0.0f);
    bool hit = hittable_collection.Hit(ray, &record);
    if (hit) {
        color = Shade(hittable_collection, record, trace_depth);
    }
    return color;
}
