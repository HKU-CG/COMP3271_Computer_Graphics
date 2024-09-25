#include "Mesh.h"
#include <iostream>

Mesh::Mesh() {
    for (int a = -1; a <= 1; a += 2)
        for (int b = -1; b <= 1; b += 2)
            for (int c = -1; c <= 1; c += 2)
                vertices_.emplace_back(1.f * a, 1.f * b, 1.f * c);

    faces_.emplace_back(0, 4, 5, 1);
    faces_.emplace_back(4, 6, 7, 5);
    faces_.emplace_back(6, 2, 3, 7);
    faces_.emplace_back(2, 0, 1, 3);
    faces_.emplace_back(1, 5, 7, 3);
    faces_.emplace_back(0, 2, 6, 4);
}

void Mesh::Draw() {
    glm::vec3 white(1.f, 1.f, 1.f);

    if (active_face_index_ >= 0) {
        for (int i = 0; i < 4; i++) {
            int vert_id = faces_[active_face_index_][i];
            vertices_[vert_id] = active_face_transform_ * glm::vec4(vertices_[vert_id], 1.f);
        }
    }

    // Draw points
    for (int k = 0; k < faces_.size(); k++) {
        const auto& face = faces_[k];
        glm::vec3 normal = glm::normalize(glm::cross(vertices_[face[1]] - vertices_[face[0]],
                                                     vertices_[face[2]] - vertices_[face[1]]));
        normal = active_transform_ * glm::vec4(normal, 0.f);
        glm::vec3 color = white * std::max(0.f, glm::dot(normal, light_direction_) * 0.8f) + white * 0.2f;
        glBegin(GL_QUADS);
        glColor3fv(&color[0]);
        for (int i = 0; i < 4; i++) {
            int vert_id = face[i];
            glVertex3fv(&(active_transform_ * glm::vec4(vertices_[vert_id], 1.f))[0]);
        }
        glEnd();

        glLineWidth(2.f);
        glColor3f(1.f, 1.f, 0.f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 4; i++) {
            int vert_id = face[i];
            glVertex3fv(&(active_transform_ * glm::vec4(vertices_[vert_id], 1.f))[0]);
        }
        glEnd();
    }

    UpdateInformation();

    // Draw local bounding box
    glLineWidth(4.f);
    glColor3f(.5f, .5f, 1.f);

    glDepthMask(GL_FALSE);
    glBegin(GL_LINES);
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 3; j++) {
            if ((i >> j & 1) == 0) {
                glVertex3fv(&local_bound_vertices_[i][0]);
                glVertex3fv(&local_bound_vertices_[i | (1 << j)][0]);
            }
        }
    }
    glEnd();
    glDepthMask(GL_TRUE);

    if (active_face_index_ >= 0) {
        glm::mat4x4 inv_mat = glm::inverse(active_face_transform_);
        for (int i = 0; i < 4; i++) {
            int vert_id = faces_[active_face_index_][i];
            vertices_[vert_id] = inv_mat * glm::vec4(vertices_[vert_id], 1.f);
        }
    }
}

void Mesh::ApplyFaceTransform(int face_index, const glm::mat4x4& transform) {
    active_face_index_ = face_index;
    active_face_transform_ = transform;
}

void Mesh::ApplyFaceTransform(int face_index, const float* transform) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            active_face_transform_[i][j] = transform[j * 4 + i];
        }
    }
}

void Mesh::ApplyTransform(const glm::mat4x4& transform) {
    active_transform_ = transform;
}

void Mesh::ApplyTransform(const float* transform) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            active_transform_[i][j] = transform[j * 4 + i];
        }
    }
}

void Mesh::CommitTransform() {
    for (glm::vec3& vertex : vertices_) {
        vertex = active_transform_ * glm::vec4(vertex, 1.f);
    }

    local_x_ = active_transform_ * glm::vec4(local_x_, 0.f);
    local_y_ = active_transform_ * glm::vec4(local_y_, 0.f);
    local_z_ = active_transform_ * glm::vec4(local_z_, 0.f);

    active_transform_ = glm::mat4x4(1.f);

    if (active_face_index_ >= 0) {
        const auto& face = faces_[active_face_index_];
        for (int i = 0; i < 4; i++) {
            vertices_[face[i]] = active_face_transform_ * glm::vec4(vertices_[face[i]], 1.f);
        }
        active_face_index_ = -1;
        active_face_transform_ = glm::mat4x4(1.f);
    }
    active_face_transform_ = glm::mat4x4(1.f);

    UpdateInformation(true);
}

void Mesh::UpdateInformation(bool commit) {
    // Update local & world bounding box information
    glm::mat3x3 base;
    glm::vec3 active_local_x = active_transform_ * glm::vec4(local_x_, 0.f);
    glm::vec3 active_local_y = active_transform_ * glm::vec4(local_y_, 0.f);
    glm::vec3 active_local_z = active_transform_ * glm::vec4(local_z_, 0.f);

    for (int i = 0; i < 3; i++) {
        base[0][i] = active_local_x[i];
        base[1][i] = active_local_y[i];
        base[2][i] = active_local_z[i];
    }

    glm::mat3x3 base_inv = glm::inverse(base);

    local_coord_min_ = glm::vec3( 1e9f,  1e9f,  1e9f);
    local_coord_max_ = glm::vec3(-1e9f, -1e9f, -1e9f);
    world_coord_min_ = glm::vec3( 1e9f,  1e9f,  1e9f);
    world_coord_max_ = glm::vec3(-1e9f, -1e9f, -1e9f);

    std::vector<glm::vec3> active_vertices = vertices_;

    for (auto& vert : active_vertices) {
        vert = active_transform_ * glm::vec4(vert, 1.f);
    }

    for (const glm::vec3& vertex : active_vertices) {
        world_coord_min_ = glm::min(world_coord_min_, vertex);
        world_coord_max_ = glm::max(world_coord_max_, vertex);
        glm::vec3 coord = base_inv * vertex;
        local_coord_min_ = glm::min(local_coord_min_, coord - glm::vec3(1e-2f, 1e-2f, 1e-2f));
        local_coord_max_ = glm::max(local_coord_max_, coord + glm::vec3(1e-2f, 1e-2f, 1e-2f));
    }

    glm::vec3 local_coords[2] = { local_coord_min_, local_coord_max_ };
    glm::vec3 world_coords[2] = { world_coord_min_, world_coord_max_ };

    for (int i = 0; i <= 1; i++) {
        for (int j = 0; j <= 1; j++) {
            for (int k = 0; k <= 1; k++) {
                local_bound_vertices_[i * 4 + j * 2 + k] =
                        local_base_ * glm::vec3(local_coords[i].x, local_coords[j].y, local_coords[k].z);
                world_bound_vertices_[i * 4 + j * 2 + k] =
                        glm::vec3(world_coords[i].x, world_coords[j].y, world_coords[k].z);
            }
        }
    }
    
    if (commit) {
        center_ = base * (local_coord_max_ + local_coord_min_) * .5f;
    }
    local_base_ = base;
}

std::tuple<int, glm::vec3> Mesh::FaceIntersection(const glm::vec3& o, const glm::vec3& d) {
    int ret_face_index = -1;
    float ret_t = 1e8f;
    glm::vec3 point;

    for (int i = 0; i < faces_.size(); i++) {
        const auto& face = faces_[i];
        glm::vec3 face_o = vertices_[face[0]];
        glm::vec3 face_n = glm::normalize(glm::cross(vertices_[face[1]] - vertices_[face[0]],
                                                          vertices_[face[2]] - vertices_[face[1]]));

        float dot_val = glm::dot(face_n, d);
        if (std::abs(dot_val) < 1e-6f) {
            continue;
        }

        float t = glm::dot(face_o - o, face_n) / dot_val;
        glm::vec3 inter_point = o + t * d;
        bool okay = true;
        for (int j = 0; j < 4; j++) {
            if (glm::dot(face_n, glm::normalize(glm::cross(vertices_[face[(j + 1) % 4]] - vertices_[face[j]],
                                                           inter_point - vertices_[face[j]]))) < 1e-3f) {
                okay = false;
                break;
            }
        }

        if (okay && (ret_t > t + 1e-5)) {
            ret_t = t;
            ret_face_index = i;
            point = inter_point;
        }
    }
    return { ret_face_index, point };
}

void Mesh::SubdivideFace(const glm::vec3 &o, const glm::vec3 &d) {
    int hit_face_index;
    glm::vec3 hit_point;
    std::tie(hit_face_index, hit_point) = FaceIntersection(o, d);
    if (hit_face_index < 0) {
        return;
    }

    int old_n_vertices = vertices_.size();
    vertices_.emplace_back(hit_point);

    Face face = faces_[hit_face_index];
    for (int i = 0; i < 4; i++) {
        const glm::vec3& b = vertices_[face[(i + 1) % 4]];
        const glm::vec3& a = vertices_[face[i]];
        const glm::vec3& v = glm::normalize(b - a);
        glm::vec3 new_vert = glm::dot(hit_point - a, v) * v + a;
        vertices_.emplace_back(new_vert);
    }

    faces_.emplace_back(face[0], old_n_vertices + 1, old_n_vertices, old_n_vertices + 4);
    faces_.emplace_back(face[1], old_n_vertices + 2, old_n_vertices, old_n_vertices + 1);
    faces_.emplace_back(face[2], old_n_vertices + 3, old_n_vertices, old_n_vertices + 2);
    faces_[hit_face_index] = Face(face[3], old_n_vertices + 4, old_n_vertices, old_n_vertices + 3);

    UpdateInformation();
}

void Mesh::GenExtrudeFace(const glm::vec3& o, const glm::vec3& d) {
    int hit_face_index;
    glm::vec3 hit_point;
    std::tie(hit_face_index, hit_point) = FaceIntersection(o, d);
    if (hit_face_index < 0) {
        return;
    }

    int old_n_vertices = vertices_.size();
    Face face = faces_[hit_face_index];

    for (int i = 0; i < 4; i++) {
        vertices_.emplace_back(vertices_[face[i]]);
    }

    faces_.emplace_back(face[0], old_n_vertices + 0, old_n_vertices + 3, face[3]);
    faces_.emplace_back(face[3], old_n_vertices + 3, old_n_vertices + 2, face[2]);
    faces_.emplace_back(face[2], old_n_vertices + 2, old_n_vertices + 1, face[1]);
    faces_.emplace_back(face[1], old_n_vertices + 1, old_n_vertices + 0, face[0]);

    faces_[hit_face_index] = Face(old_n_vertices, old_n_vertices + 1, old_n_vertices + 2, old_n_vertices + 3);
}
