#include "Mesh.h"
#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION

#include "tiny_obj_loader.h"

bool loadOBJ(const char *path, std::vector<glm::vec3> &vertices, std::vector<unsigned int> &indices,
             std::vector<glm::vec3> &normals, std::vector<glm::vec2> &texCoords) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;
    std::cout << "Loading " << path << std::endl;
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path)) {
        std::cerr << warn << err << std::endl;
        return false;
    }
    for (const auto &shape: shapes) {
        for (const auto &index: shape.mesh.indices) {
            glm::vec3 vertex(
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
            );
            glm::vec3 normal(
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
            );
            glm::vec2 texCoord(
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]
            );
            texCoords.push_back(texCoord);
            vertices.push_back(vertex);
            normals.push_back(normal);
            indices.push_back(indices.size());
        }
    }

    std::cout << "Loaded " << vertices.size() << " vertices" << std::endl;
    return true;
}

Mesh::Mesh() {
    if (!loadOBJ("../bunny.obj", vertices_, indices_, normals_, texCoords_)) {
        std::cerr << "Failed to load OBJ file" << std::endl;
    }
}

void Mesh::Draw(unsigned int VAO, unsigned int shaderProgram, size_t indexCount) {
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(active_transform_));
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::ApplyTransform(const glm::mat4x4 &transform) {
    active_transform_ = transform;
}

void Mesh::setupBuffers(unsigned int &VAO, unsigned int &VBO, unsigned int &NBO, unsigned int &EBO, unsigned int &TBO) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &NBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &TBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(glm::vec3), vertices_.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *) 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, NBO);
    glBufferData(GL_ARRAY_BUFFER, normals_.size() * sizeof(glm::vec3), normals_.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *) 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, TBO);
    glBufferData(GL_ARRAY_BUFFER, texCoords_.size() * sizeof(glm::vec2), texCoords_.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *) 0);
    glEnableVertexAttribArray(2);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned int), indices_.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

