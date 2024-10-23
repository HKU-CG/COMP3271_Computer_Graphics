#pragma once
#include "Common.h"

class Camera {
public:
    // ---------------
    // Main functions
    // ---------------
    void LookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up);
    void Perspective(float fovy, float aspect, float near, float far);
    void UpdateScale(float scale);
    void ApplyProjection();
    void ApplyTransform(const glm::mat4x4& transform);

    // Data
    glm::mat4x4 view_mat_;          // The view transform matrix 
    glm::mat4x4 view_mat_inv_;
    glm::mat4x4 projection_mat_;    // The projection matrix
    glm::mat4x4 projection_mat_inv_;
    glm::vec3 eye_;
    float fov_;
};
