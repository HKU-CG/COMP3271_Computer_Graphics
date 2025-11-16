#pragma once
#include <random>
// GLM
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/scalar_constants.hpp> // glm::pi

using Color = glm::vec3;
using Vec = glm::vec3;
using Point = glm::vec3;

#undef M_PI
#define M_PI 3.141592653589793f

inline float get_random_float()
{
    static std::random_device dev;
    std::mt19937 rng(dev());
    static std::uniform_real_distribution<float> dist(0.f, 1.f); // distribution in range [0, 1]

    return dist(rng);
}
