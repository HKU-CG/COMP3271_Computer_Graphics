#include "Hittable.h"
#include "as3.h"

// Sphere
bool Sphere::Hit(const Ray &ray, HitRecord *hit_record) const {
    return sphere_hit(ray, hit_record, o_, r_, material_);
}

// Quadric
bool Quadric::Hit(const Ray &ray, HitRecord *hit_record) const {

    glm::vec4 D(ray.d, 0.f);
    glm::vec4 S(ray.o, 1.f);

    float a = glm::dot(D, A_ * D);
    float b = 2.f * glm::dot(S, A_ * D);
    float c = glm::dot(S, A_ * S);
    float delta = b * b - 4.f * a * c;

    if (delta < 1e-5f) {
        return false;
    }

    float t = glm::min((-b - glm::sqrt(delta)) / (2.f * a), (-b + glm::sqrt(delta)) / (2.f * a));
    if (t < 1e-3f) {
        return false;
    }

    Point hit_pos = ray.o + ray.d * t;
    Vec normal = glm::normalize(glm::vec3((A_ + glm::transpose(A_)) * glm::vec4(hit_pos, 1.f)));
    hit_record->position = hit_pos;
    hit_record->normal = normal;
    hit_record->distance = t;
    hit_record->in_direction = ray.d;
    hit_record->reflection = ray.d - 2.f * glm::dot(ray.d, hit_record->normal) * hit_record->normal;
    hit_record->material = material_;
    return true;

}

// Triangle
bool Triangle::Hit(const Ray &ray, HitRecord *hit_record) const {
    return triangle_hit(ray, hit_record, a_, b_, c_, n_a_, n_b_, n_c_);
}

// ---------------------------------------------------------------------------------------------
// ------------------------------ no need to change --------------------------------------------
// ---------------------------------------------------------------------------------------------

// CompleteTriangle
bool CompleteTriangle::Hit(const Ray &ray, HitRecord *hit_record) const {
    bool ret = triangle_.Hit(ray, hit_record);
    if (ret) {
        hit_record->material = material_;
    }
    return ret;
}


// Mesh
Mesh::Mesh(const std::string &file_path,
           const Material &material,
           bool phong_interpolation) :
        ply_data_(file_path), material_(material), phong_interpolation_(phong_interpolation) {
    std::vector<std::array<double, 3>> v_pos = ply_data_.getVertexPositions();
    vertices_.resize(v_pos.size());

    for (int i = 0; i < vertices_.size(); i++) {
        vertices_[i] = Point(v_pos[i][0], v_pos[i][1], v_pos[i][2]);
    }

    f_ind_ = ply_data_.getFaceIndices();

    // Calc face normals
    for (const auto &face: f_ind_) {
        Vec normal = glm::normalize(
                glm::cross(vertices_[face[1]] - vertices_[face[0]], vertices_[face[2]] - vertices_[face[0]]));
        face_normals_.emplace_back(normal);
    }

    // Calc vertex normals
    vertex_normals_.resize(vertices_.size(), Vec(0.f, 0.f, 0.f));
    for (int i = 0; i < f_ind_.size(); i++) {
        for (int j = 0; j < 3; j++) {
            vertex_normals_[f_ind_[i][j]] += face_normals_[i];
        }
    }
    for (auto &vertex_normal: vertex_normals_) {
        vertex_normal = glm::normalize(vertex_normal);
    }

    // Construct hittable triangles
    for (const auto &face: f_ind_) {
        triangles_.emplace_back(vertices_[face[0]], vertices_[face[1]], vertices_[face[2]],
                                vertex_normals_[face[0]], vertex_normals_[face[1]], vertex_normals_[face[2]],
                                phong_interpolation_);
    }

    // Calc bounding box
    Point bbox_min(1e5f, 1e5f, 1e5f);
    Point bbox_max(-1e5f, -1e5f, -1e5f);
    for (const auto &vertex: vertices_) {
        bbox_min = glm::min(bbox_min, vertex - 1e-3f);
        bbox_max = glm::max(bbox_max, vertex + 1e-3f);
    }

    // Build Octree
    tree_nodes_.emplace_back(new OctreeNode());
    tree_nodes_.front()->bbox_min = bbox_min;
    tree_nodes_.front()->bbox_max = bbox_max;

    root_ = tree_nodes_.front().get();
    for (int i = 0; i < f_ind_.size(); i++) {
        InsertFace(root_, i);
    }

    area = 0.0f;
    for (auto &triangle: triangles_) {
        area += triangle.getArea();
    }
    emission = material_.emission;
}

bool Mesh::Hit(const Ray &ray, HitRecord *hit_record) const {
    const bool brute_force = false;
    if (brute_force) {
        // Naive hit algorithm
        float min_dist = 1e5f;
        for (const auto &triangle: triangles_) {
            HitRecord curr_hit_record;
            if (triangle.Hit(ray, &curr_hit_record)) {
                if (curr_hit_record.distance < min_dist) {
                    *hit_record = curr_hit_record;
                    min_dist = curr_hit_record.distance;
                }
            }
        }
        if (min_dist + 1.0 < 1e5f) {
            hit_record->material = material_;
            return true;
        }
        return false;
    } else {
        bool ret = OctreeHit(root_, ray, hit_record);
        if (ret) {
            hit_record->material = material_;
        }
        return ret;
    }
}

bool Mesh::IsFaceInsideBox(const std::vector<size_t> &face, const Point &bbox_min, const Point &bbox_max) const {
    for (size_t idx: face) {
        const auto &pt = vertices_[idx];
        for (int i = 0; i < 3; i++) {
            if (pt[i] < bbox_min[i] + 1e-6f) return false;
            if (pt[i] > bbox_max[i] - 1e-6f) return false;
        }
    }
    return true;
}

bool Mesh::IsRayIntersectBox(const Ray &ray, const Point &bbox_min, const Point &bbox_max) const {
    float t_min = -1e5f;
    float t_max = 1e5f;

    for (int i = 0; i < 3; i++) {
        if (glm::abs(ray.d[i]) < 1e-6f) {
            if (ray.o[i] < bbox_min[i] + 1e-6f || ray.o[i] > bbox_max[i] - 1e-6f) {
                t_min = 1e5f;
                t_max = -1e5f;
            }
        } else {
            if (ray.d[i] > 0.f) {
                t_min = glm::max(t_min, (bbox_min[i] - ray.o[i]) / ray.d[i]);
                t_max = glm::min(t_max, (bbox_max[i] - ray.o[i]) / ray.d[i]);
            } else {
                t_min = glm::max(t_min, (bbox_max[i] - ray.o[i]) / ray.d[i]);
                t_max = glm::min(t_max, (bbox_min[i] - ray.o[i]) / ray.d[i]);
            }
        }
    }

    return t_min + 1e-6f < t_max;
}

void Mesh::InsertFace(OctreeNode *u, size_t face_idx) {
    const Point &bbox_min = u->bbox_min;
    const Point &bbox_max = u->bbox_max;

    Vec bias = bbox_max - bbox_min;
    Vec half_bias = bias * 0.5f;

    bool inside_childs = false;

    for (size_t a = 0; a < 2; a++) {
        for (size_t b = 0; b < 2; b++) {
            for (size_t c = 0; c < 2; c++) {
                size_t child_idx = ((a << 2) | (b << 1) | c);
                Point curr_bbox_min = bbox_min + half_bias * Vec(float(a), float(b), float(c));
                Point curr_bbox_max = curr_bbox_min + half_bias;
                if (IsFaceInsideBox(f_ind_[face_idx], curr_bbox_min, curr_bbox_max)) {
                    if (u->childs[child_idx] == nullptr) {
                        tree_nodes_.emplace_back(new OctreeNode());
                        OctreeNode *child = tree_nodes_.back().get();
                        u->childs[child_idx] = tree_nodes_.back().get();
                        child->bbox_min = curr_bbox_min;
                        child->bbox_max = curr_bbox_max;
                    }
                    InsertFace(u->childs[child_idx], face_idx);
                    inside_childs = true;
                }
            }
        }
    }

    if (!inside_childs) {
        u->face_index.push_back(face_idx);
    }
}

bool Mesh::OctreeHit(OctreeNode *u, const Ray &ray, HitRecord *hit_record) const {
    if (!IsRayIntersectBox(ray, u->bbox_min, u->bbox_max)) {
        return false;
    }
    float distance = 1e5f;
    for (const auto &face_idx: u->face_index) {
        HitRecord curr_hit_record;
        if (triangles_[face_idx].Hit(ray, &curr_hit_record)) {
            if (curr_hit_record.distance < distance) {
                distance = curr_hit_record.distance;
                *hit_record = curr_hit_record;
            }
        }
    }

    for (const auto &child: u->childs) {
        if (child == nullptr) {
            continue;
        }
        HitRecord curr_hit_record;
        if (OctreeHit(child, ray, &curr_hit_record)) {
            if (curr_hit_record.distance < distance) {
                distance = curr_hit_record.distance;
                *hit_record = curr_hit_record;
            }
        }
    }
    return distance + 1 < 1e5f;
}


// Hittable list
void HittableList::PushHittable(const Hittable &hittable) {
    hittable_list_.push_back(&hittable);
}

bool HittableList::Hit(const Ray &ray, HitRecord *hit_record) const {
    float min_dist = 1e5f;
    for (const auto &hittable: hittable_list_) {
        HitRecord curr_hit_record;
        if (hittable->Hit(ray, &curr_hit_record)) {
            if (curr_hit_record.distance < min_dist) {
                *hit_record = curr_hit_record;
                min_dist = curr_hit_record.distance;
            }
        }
    }
    return min_dist + 1.0 < 1e4f;
}
