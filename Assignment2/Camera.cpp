#include "Camera.h"
#include <cstring>

void Camera::LookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up) {
    view_mat_ = glm::lookAt(eye, center, up);
    view_mat_inv_ = glm::inverse(view_mat_);
    eye_ = eye;
}

void Camera::Perspective(float fovy, float aspect, float near, float far) {
    fov_ = fovy;
    projection_mat_ = glm::perspective(glm::radians(fovy), aspect, near, far);
    projection_mat_inv_ = glm::inverse(projection_mat_);
}

void Camera::UpdateScale(float scale) {
    glm::mat4x4 transform = glm::scale(glm::mat4x4(1.f), { scale, scale, scale });
    view_mat_ = view_mat_ * transform;
    view_mat_inv_ = glm::inverse(view_mat_);
}

void Camera::ApplyProjection() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMultMatrixf((GLfloat*) &view_mat_[0][0]);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMultMatrixf((GLfloat*) &projection_mat_[0][0]);
}

void Camera::ApplyTransform(const glm::mat4x4& transform) {
    view_mat_ = view_mat_ * transform;
    view_mat_inv_ = glm::inverse(view_mat_);
}