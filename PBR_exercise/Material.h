#pragma once
#include "Common.h"
#include "Ray.h"


// Simple material
// Reference: https://en.wikipedia.org/wiki/Phong_reflection_model
struct Material {
    Material() = default;

    Material(const Color &diffuse,
             const Color &specular,
             float k_d,
             float k_s,
             float roughness,
             float emission) : diffuse(diffuse), specular(specular), k_d(k_d), k_s(k_s), roughness(roughness),
                               emission(emission) {
    }

    Color diffuse, specular;
    float k_d, k_s;
    float roughness = 1.f;
    float emission = 0.f;
};
