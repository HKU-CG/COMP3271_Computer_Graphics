#include <iostream>
#include <vector>
#include <thread>
#include "Common.h"
#include "Scene.h"
#include "Camera.h"
#include "Material.h"
#include "Hittable.h"
#include "Utils/lodepng.h"

// TODO: feel free to modify the following parameters
const int kMaxTraceDepth = 5;
const int spp = 8;
const int NUM_THREAD = 8;
const std::string scene_file = "scene/teapot_area_light.toml";
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

void thread_func(int thread_i, int n_thread, int spp, int width, int height, const Camera &camera, const Scene &scene,
                 float *image_buffer) {
    auto now = std::chrono::high_resolution_clock::now();
    int thread_spp = spp / n_thread;
    float progress = 0.f;

    for (int x = thread_i; x < width; x += n_thread) {
        for (int y = 0; y < height; ++y) {
            tracePixel(x, y, width, height, thread_spp, camera, scene, image_buffer);
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

    std::vector<std::thread> thread_pool;
    for (int thread_i = 0; thread_i < NUM_THREAD; ++thread_i) {
        thread_pool.emplace_back(thread_func, thread_i, NUM_THREAD, spp, width, height, std::ref(camera),
                                 std::ref(scene), image_buffer);
    }
    for (std::thread &t: thread_pool) {
        t.join();
    }

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

Vec SampleHemisphere(const Vec &normal) {

    auto ray = Vec(0.0f, 0.0f, 0.0f);
    // TODO 3: add your code here.
    // This function randomly samples a direction on the hemisphere.
    // Hint: sample a random direction on the hemisphere with normal as the z-axis
    // Hint: Use Rodrigues' rotation formula to rotate the z-axis to the normal direction
    return ray;
}

Vec samplePhongLobe(const Vec &reflectionDir, double shininess) {
    float u1 = get_random_float();
    float u2 = get_random_float();

    float phi = 2 * M_PI * u1;
    float cosTheta = pow(u2, 1.0 / (shininess + 1));
    float sinTheta = sqrt(1 - cosTheta * cosTheta);

    float x = sinTheta * cos(phi);
    float y = sinTheta * sin(phi);
    float z = cosTheta;

    // Build orthonormal basis around reflectionDir
    Vec w = reflectionDir; // Note: reflectionDir should be normalized
    Vec up = fabs(w.z) < 0.999 ? Vec(0, 0, 1) : Vec(1, 0, 0);
    Vec u = glm::normalize(glm::cross(up, w));
    Vec v = glm::cross(w, u);

    // Convert to world coordinates
    Vec newDir = glm::normalize(u * x + v * y + w * z);
    return newDir;
}

Color Shade(const Hittable &hittable_collection,
            const HitRecord &hit_record,
            int trace_depth) {
    Color color(0.f, 0.f, 0.f);

    const Material &material = hit_record.material;
    Point o = hit_record.position;
    color = material.ambient * material.k_a;

    // 1. If the object has emission, add its contribution to the color.
    color += material.emission;
    if (material.k_d > 0.f && trace_depth < kMaxTraceDepth) {
        // TODO 4: implement the diffuse reflection part of the shader.
        Vec wi = SampleHemisphere(hit_record.normal);
        // color += diffuseReflectance * BRDF * cos_theta / pdf;
    }
    // 3. calculate specular
    if (material.k_s > 0.f && trace_depth < kMaxTraceDepth) {
        // Glossy specular reflection using Phong BRDF
        Vec reflection = glm::normalize(glm::reflect(hit_record.in_direction, hit_record.normal));
        // Sample around the reflection direction
        Vec newDir = samplePhongLobe(reflection, material.sh);
        float cosAlpha = glm::dot(newDir, reflection);
        if (cosAlpha < 0) cosAlpha = 0;
        // BRDF and PDF for Phong reflection
        Vec brdf = material.specular * ((material.sh + 2) / (2 * M_PI)) * (float) pow(cosAlpha, material.sh);
        float pdf = ((material.sh + 1) / (2 * M_PI)) * pow(cosAlpha, material.sh);
        Vec reflectColor = TraceRay(Ray(hit_record.position, newDir), hittable_collection, trace_depth + 1);

        float cosine = glm::dot(hit_record.normal, hit_record.reflection);
        color += material.k_s * reflectColor * brdf / pdf * cosine;
    }

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
