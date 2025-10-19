#pragma once

#include <vector>
#include <tuple>
#include "Common.h"

// -----------------------------------------------------------------------------
// Mesh: The class that keeps the geometric information
// -----------------------------------------------------------------------------
class Mesh {
public:
    // ----------------
    // Main functions
    // ----------------
    Mesh();

    // Public members
    std::vector<glm::vec3> vertices_;
    std::vector<glm::vec3> normals_;
    std::vector<glm::vec2> texCoords_;
    std::vector<unsigned int> indices_;
    glm::vec3 center_ = glm::vec3(0.f, 0.f, 0.f);

    // No need to use the following members
public:
    void setupBuffers(unsigned int &VAO, unsigned int &VBO, unsigned int &NBO, unsigned int &EBO,unsigned int &TBO);

    // Apply transform to all mesh vertices.
    // Params:
    //     transform: the 4x4 transformation matrix.
    void ApplyTransform(const glm::mat4x4& transform);

    void Draw(unsigned int VAO, unsigned int shaderProgram, size_t indexCount);

private:

    glm::mat4x4 active_transform_ = glm::mat4x4(1.f);
};
