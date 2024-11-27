#include "Scene.h"

Scene::Scene(const std::string& work_dir, const std::string& scene_file_name) {
    auto scene_data = toml::parse(work_dir + scene_file_name);
    // load camera
    auto mat = toml::find<std::vector<std::vector<float>>>(scene_data["camera"], "extrinsic");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            camera_.extrinsic_[i][j] = mat[i][j];
        }
    }

    camera_.extrinsic_inv_ = glm::inverse(camera_.extrinsic_);
    camera_.width_ = toml::find<int>(scene_data["camera"], "width");
    camera_.height_ = toml::find<int>(scene_data["camera"], "height");
    float fov = toml::find<float>(scene_data["camera"], "fov");
    camera_.fovy_ = fov;
    camera_.focal_length_ = 1.f / glm::tan(glm::radians(fov / 2.f)) * float(camera_.height_) * 0.5f;


    // load hittable objects
    int hittable_size = scene_data["hittable"].size();
    for (int i = 0; i < hittable_size; i++) {
        // load material
        auto diffuse_vec =  toml::find<std::vector<float>>(scene_data["hittable"][i], "diffuse");
        auto specular_vec = toml::find<std::vector<float>>(scene_data["hittable"][i], "specular");
        Color diffuse(diffuse_vec[0], diffuse_vec[1], diffuse_vec[2]);
        Color specular(specular_vec[0], specular_vec[1], specular_vec[2]);

        float k_d = toml::find<float>(scene_data["hittable"][i], "k_d");
        float k_s = toml::find<float>(scene_data["hittable"][i], "k_s");
        float roughness = toml::find<float>(scene_data["hittable"][i], "roughness");
        float emission = 0.f;
        try {
            emission = toml::find<float>(scene_data["hittable"][i], "emission");
        }
        catch (const std::exception& e) {
			emission = 0.f;
		}
        

        material_pool_.emplace_back(new Material(diffuse, specular, k_d, k_s, roughness, emission));
        const Material& curr_material = *(material_pool_.back());

        // load hittable: core
        std::string type = toml::find<std::string>(scene_data["hittable"][i], "type");
        if (type == "sphere") {
            auto pos_vec = toml::find<std::vector<float>>(scene_data["hittable"][i], "position");
            Point pos(pos_vec[0], pos_vec[1], pos_vec[2]);
            float radius = toml::find<float>(scene_data["hittable"][i], "radius");

            hittable_pool_.emplace_back((Hittable*) new Sphere(pos, radius, curr_material));
        }
        else if (type == "quadric") {
            glm::mat4x4 A;
            auto A_vec = toml::find<std::vector<std::vector<float>>>(scene_data["hittable"][i], "A");
            for (int u = 0; u < 4; u++) {
                for (int v = 0; v < 4; v++) {
                    A[u][v] = A_vec[u][v];
                }
            }

            hittable_pool_.emplace_back((Hittable*) new Quadric(A, curr_material));
        }
        else if (type == "triangle") {
            auto a_vec = toml::find<std::vector<float>>(scene_data["hittable"][i], "a");
            auto b_vec = toml::find<std::vector<float>>(scene_data["hittable"][i], "b");
            auto c_vec = toml::find<std::vector<float>>(scene_data["hittable"][i], "c");
            auto n_a_vec = toml::find<std::vector<float>>(scene_data["hittable"][i], "n_a");
            auto n_b_vec = toml::find<std::vector<float>>(scene_data["hittable"][i], "n_b");
            auto n_c_vec = toml::find<std::vector<float>>(scene_data["hittable"][i], "n_c");

            Point a(a_vec[0], a_vec[1], a_vec[2]);
            Point b(b_vec[0], b_vec[1], b_vec[2]);
            Point c(c_vec[0], c_vec[1], c_vec[2]);
            Vec n_a(n_a_vec[0], n_a_vec[1], n_a_vec[2]);
            Vec n_b(n_b_vec[0], n_b_vec[1], n_b_vec[2]);
            Vec n_c(n_c_vec[0], n_c_vec[1], n_c_vec[2]);

            hittable_pool_.emplace_back((Hittable*) new CompleteTriangle(a, b, c, n_a, n_b, n_c, curr_material));
        }
        else if (type == "mesh") {
            std::string file_path = work_dir + toml::find<std::string>(scene_data["hittable"][i], "file_path");
            bool phong_interpolation = toml::find<bool>(scene_data["hittable"][i], "phong_interpolation");
            hittable_pool_.emplace_back((Hittable*) new Mesh(file_path, curr_material, phong_interpolation));
        }

        hittable_collection_.PushHittable(*(hittable_pool_.back()));
    }
}