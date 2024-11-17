#pragma once
#include "Common.h"
#include "Hittable.h"

// Camera: Rigid transform + perspective transform
class Camera {
public:
    Camera() = default;

    Camera(const glm::mat4x4 &extrinsic, float fovy, int width, int height) {
        extrinsic_ = extrinsic;
        extrinsic_inv_ = glm::inverse(extrinsic);
        fovy_ = fovy;
        width_ = width;
        height_ = height;
        focal_length_ = 1.f / glm::tan(glm::radians(fovy / 2.f)) * float(height) * 0.5f;
    }

    Ray RayAt(float x, float y) const {
        x = (x - float(width_) *  0.5f) / focal_length_;
        y = (y - float(height_) * 0.5f) / focal_length_;
        float z = -1.f;
        Vec direction(x, y, z);
        direction = glm::normalize(direction);
        Vec position(0.f, 0.f, 0.f);

        // Camera to world
        position = extrinsic_inv_ * glm::vec4(position, 1.f);
        direction = extrinsic_inv_ * glm::vec4(direction, 0.f);

        return { position, direction };
    }

public:
    glm::mat4x4 extrinsic_, extrinsic_inv_;
    int width_, height_;
    float fovy_, focal_length_;
};