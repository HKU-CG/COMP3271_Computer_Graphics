#include "assignment2_app.hpp"

#include "gl_base.hpp"

#include <imgui.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr int kCoreViews = 3;
constexpr int kShadowView = 3;
constexpr int kViewCount = 4;
constexpr int kShadowSize = 2048;
constexpr std::array<const char *, kViewCount> kModes = {
    "1. Normals (20)", "2. Blinn-Phong (50)", "3. Texture (30)", "Shadow mapping (+5)"};
constexpr std::array<const char *, kViewCount> kShaderNames = {
    "normal", "blinn_phong", "texture", "shadow"};
constexpr float kPi = 3.14159265358979323846F;

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

MeshData load_bunny() {
    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig config;
    config.triangulate = true;
    if (!reader.ParseFromFile("asset/bunny.obj", config)) {
        throw std::runtime_error("Cannot load asset/bunny.obj: " + reader.Error());
    }
    const auto &attrib = reader.GetAttrib();
    const std::size_t count = attrib.vertices.size() / 3;
    if (count == 0) { throw std::runtime_error("Bunny OBJ has no vertices"); }
    std::vector<glm::vec3> positions(count);
    glm::vec3 lower(1e30F), upper(-1e30F);
    for (std::size_t i = 0; i < count; ++i) {
        // Convert the supplied Z-up OBJ to the course's Y-up convention.
        const glm::vec3 p(
            attrib.vertices[3 * i], attrib.vertices[3 * i + 2], -attrib.vertices[3 * i + 1]);
        positions[i] = p;
        lower = glm::min(lower, p);
        upper = glm::max(upper, p);
    }
    if (upper.y - lower.y < 1e-6F) { throw std::runtime_error("Bunny OBJ has no height"); }
    const glm::vec3 center = (lower + upper) * 0.5F;
    for (auto &p : positions) { p = (p - center) * (2.1F / (upper.y - lower.y)); }

    std::vector<unsigned int> indices;
    std::vector<glm::vec3> normals(count, glm::vec3(0.0F));
    for (const auto &shape : reader.GetShapes()) {
        std::size_t offset = 0;
        for (unsigned char corners : shape.mesh.num_face_vertices) {
            if (corners != 3) { throw std::runtime_error("Bunny contains a non-triangle face"); }
            std::array<unsigned int, 3> face{};
            for (int corner = 0; corner < 3; ++corner) {
                const int index = shape.mesh.indices[offset + corner].vertex_index;
                if (index < 0 || static_cast<std::size_t>(index) >= count) {
                    throw std::runtime_error("Bunny has an invalid vertex index");
                }
                face[corner] = static_cast<unsigned int>(index);
                indices.push_back(face[corner]);
            }
            const glm::vec3 n = glm::cross(
                positions[face[1]] - positions[face[0]], positions[face[2]] - positions[face[0]]);
            for (unsigned int index : face) { normals[index] += n; }
            offset += corners;
        }
    }
    if (indices.empty()) { throw std::runtime_error("Bunny OBJ has no faces"); }
    std::vector<Vertex> base(count);
    for (std::size_t i = 0; i < count; ++i) {
        const auto &p = positions[i];
        const float r = std::max(glm::length(p), 1e-6F);
        base[i] = {
            p,
            glm::length(normals[i]) > 1e-8F ? glm::normalize(normals[i]) : glm::vec3(0, 1, 0),
            {0.5F + std::atan2(p.z, p.x) / (2.0F * kPi),
                                                                                            0.5F + std::asin(std::clamp(p.y / r, -1.0F, 1.0F)) / kPi}
        };
    }
    MeshData mesh;
    // Split the longitude seam so UV interpolation stays local.
    constexpr unsigned int missing = std::numeric_limits<unsigned int>::max();
    std::vector<std::array<unsigned int, 2>> remap(count, {missing, missing});
    for (std::size_t i = 0; i < indices.size(); i += 3) {
        const float low =
            std::min({base[indices[i]].uv.x, base[indices[i + 1]].uv.x, base[indices[i + 2]].uv.x});
        const float high =
            std::max({base[indices[i]].uv.x, base[indices[i + 1]].uv.x, base[indices[i + 2]].uv.x});
        for (int corner = 0; corner < 3; ++corner) {
            const unsigned int index = indices[i + corner];
            const bool shifted = high - low > 0.5F && base[index].uv.x < 0.5F;
            unsigned int &destination = remap[index][shifted ? 1 : 0];
            if (destination == missing) {
                destination = static_cast<unsigned int>(mesh.vertices.size());
                Vertex vertex = base[index];
                if (shifted) { vertex.uv.x += 1.0F; }
                mesh.vertices.push_back(vertex);
            }
            mesh.indices.push_back(destination);
        }
    }
    return mesh;
}

MeshData make_plane() {
    // A clean UV patch for texture filtering checks.
    return {
        {{{-1, -1, 0}, {0, 0, 1}, {0, 0}},
         {{1, -1, 0}, {0, 0, 1}, {1, 0}},
         {{1, 1, 0}, {0, 0, 1}, {1, 1}},
         {{-1, 1, 0}, {0, 0, 1}, {0, 1}}},
        {0, 1, 2, 0, 2, 3}
    };
}

class Mesh {
  public:
    explicit Mesh(MeshData mesh) {
        count_ = static_cast<GLsizei>(mesh.indices.size());
        vao_.bind();
        vbo_.set_data(mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);
        ebo_.set_data(
            mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW,
            GL_ELEMENT_ARRAY_BUFFER);
        vbo_.bind(GL_ARRAY_BUFFER);
        vao_.set_element_buffer(ebo_);
        vao_.set_attrib(
            0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            reinterpret_cast<const void *>(offsetof(Vertex, position)));
        vao_.set_attrib(
            1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            reinterpret_cast<const void *>(offsetof(Vertex, normal)));
        vao_.set_attrib(
            2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            reinterpret_cast<const void *>(offsetof(Vertex, uv)));
        GLVertexArray::unbind();
        GLBuffer::unbind(GL_ARRAY_BUFFER);
    }

    void draw() const {
        vao_.bind();
        glDrawElements(GL_TRIANGLES, count_, GL_UNSIGNED_INT, nullptr);
        GLVertexArray::unbind();
    }

  private:
    GLVertexArray vao_;
    GLBuffer vbo_;
    GLBuffer ebo_;
    GLsizei count_ = 0;
};

void set_repeat_mipmaps(GLTexture2D &texture) {
    texture.bind();
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GLTexture2D::unbind();
}

void create_checker_texture(GLTexture2D &checker) {
    constexpr int size = 128, levels = 8;
    std::vector<unsigned char> color(size * size * 4);
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            const bool bright = ((x / 16) + (y / 16)) % 2 == 0;
            const int offset = (y * size + x) * 4;
            color[offset] = bright ? 226 : 40;
            color[offset + 1] = bright ? 177 : 97;
            color[offset + 2] = bright ? 105 : 130;
            color[offset + 3] = 255;
        }
    }
    checker.storage(levels, GL_RGBA8, size, size);
    checker.sub_image(0, 0, 0, size, size, GL_RGBA, GL_UNSIGNED_BYTE, color.data());
    set_repeat_mipmaps(checker);
}

void allocate_depth(GLTexture2D &texture, int width, int height) {
    // The shared storage helper uses RGBA upload data, so allocate depth explicitly.
    texture.bind();
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT,
        GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    const float border[] = {1.0F, 1.0F, 1.0F, 1.0F};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    GLTexture2D::unbind();
}

} // namespace

class Assignment2App::Impl {
  public:
    Impl() {
        create_checker_texture(checker_);
        allocate_depth(shadow_depth_, kShadowSize, kShadowSize);
        shadow_framebuffer_.attach_texture(GL_DEPTH_ATTACHMENT, shadow_depth_);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        if (!shadow_framebuffer_.complete()) {
            throw std::runtime_error("Shadow framebuffer is incomplete");
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        reload_shaders();
    }

    void draw() {
        const ImGuiIO &io = ImGui::GetIO();
        if (spin_) { angle_ = std::remainder(angle_ + io.DeltaTime * 0.55F, 2.0F * kPi); }
        controls();
        const float left = 350.0F;
        ImGui::SetNextWindowPos({left, 0.0F});
        ImGui::SetNextWindowSize({std::max(1.0F, io.DisplaySize.x - left), io.DisplaySize.y});
        ImGui::Begin(
            "Shader Lab", nullptr,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse
                | ImGuiWindowFlags_NoBringToFrontOnFocus);
        if (ImGui::BeginTabBar("Lab sections")) {
            if (ImGui::BeginTabItem("Required (100 points)")) {
                const ImVec2 available = ImGui::GetContentRegionAvail();
                const ImVec2 spacing = ImGui::GetStyle().ItemSpacing;
                const ImVec2 cell{
                    std::max(1.0F, (available.x - spacing.x) * 0.5F),
                    std::max(1.0F, (available.y - spacing.y) * 0.5F)};
                for (int i = 0; i < kCoreViews; ++i) {
                    if (i % 2 == 1) { ImGui::SameLine(); }
                    ImGui::PushID(i);
                    const ImVec2 view_size{i == kCoreViews - 1 ? available.x : cell.x, cell.y};
                    ImGui::BeginChild("view", view_size, ImGuiChildFlags_Borders);
                    show_view(i);
                    ImGui::EndChild();
                    ImGui::PopID();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Shadow bonus (+5)")) {
                render_shadow_depth();
                ImGui::TextWrapped("A directional light casts the bunny's shadow onto the ground.");
                show_view(kShadowView);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    }

  private:
    void show_view(int index) {
        const ImGuiIO &io = ImGui::GetIO();
        ImGui::TextUnformatted(kModes[index]);
        ImVec2 size = ImGui::GetContentRegionAvail();
        size.x = std::max(1.0F, size.x);
        size.y = std::max(1.0F, size.y);
        const int width = std::max(1, static_cast<int>(size.x * io.DisplayFramebufferScale.x));
        const int height = std::max(1, static_cast<int>(size.y * io.DisplayFramebufferScale.y));
        ensure_target_size(index, width, height);
        render_view(index, width, height);
        ImGui::Image(
            static_cast<ImTextureID>(static_cast<unsigned int>(colors_[index])), size, {0.0F, 1.0F},
            {1.0F, 0.0F});
        if (ImGui::IsItemHovered()) {
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                yaw_ += io.MouseDelta.x * 0.01F;
                pitch_ = std::clamp(pitch_ - io.MouseDelta.y * 0.01F, -1.4F, 1.4F);
            }
            radius_ = std::clamp(radius_ - io.MouseWheel * 0.25F, 2.0F, 9.0F);
        }
    }

    void reload_shaders() {
        for (int i = 0; i < kViewCount; ++i) {
            const std::string base = std::string("Assignment2/shaders/") + kShaderNames[i];
            try {
                Shader next(base + ".vert", base + ".frag");
                shaders_[i] = std::move(next);
                shader_errors_[i].clear();
            } catch (const std::exception &error) { shader_errors_[i] = error.what(); }
        }
        try {
            Shader next(
                "Assignment2/shaders/shadow_depth.vert", "Assignment2/shaders/shadow_depth.frag");
            depth_shader_ = std::move(next);
            depth_error_.clear();
        } catch (const std::exception &error) { depth_error_ = error.what(); }
    }

    void controls() {
        const ImGuiIO &io = ImGui::GetIO();
        ImGui::SetNextWindowPos({0.0F, 0.0F});
        ImGui::SetNextWindowSize({350.0F, io.DisplaySize.y});
        ImGui::Begin(
            "Assignment 2", nullptr,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::TextUnformatted("Shader Lab: 100 + 5 points");
        if (ImGui::Button("Reload all shaders")) { reload_shaders(); }
        for (int i = 0; i < kViewCount; ++i) {
            if (!shader_errors_[i].empty()) {
                ImGui::TextColored({1.0F, 0.45F, 0.4F, 1.0F}, "%s error:", kModes[i]);
                ImGui::TextWrapped("%s", shader_errors_[i].c_str());
            }
        }
        if (!depth_error_.empty()) { ImGui::TextWrapped("Depth shader: %s", depth_error_.c_str()); }
        ImGui::Separator();
        ImGui::PushItemWidth(150.0F);
        ImGui::TextUnformatted("Model");
        ImGui::SetNextItemWidth(-1.0F);
        ImGui::Combo("##Model", &model_choice_, "Stanford Bunny\0Detail plane\0");
        ImGui::Checkbox("Spin model", &spin_);
        ImGui::Checkbox("Nonuniform scale", &nonuniform_scale_);
        ImGui::SliderFloat("Model rotation", &angle_, -kPi, kPi);
        if (ImGui::CollapsingHeader("Camera")) {
            ImGui::SliderFloat("Yaw", &yaw_, -kPi, kPi);
            ImGui::SliderFloat("Pitch", &pitch_, -1.4F, 1.4F);
            ImGui::SliderFloat("Distance", &radius_, 2.0F, 9.0F);
            if (ImGui::Button("Reset camera")) {
                yaw_ = 0.6F;
                pitch_ = 0.18F;
                radius_ = 4.0F;
            }
        }
        ImGui::Separator();
        ImGui::TextUnformatted("Point light");
        ImGui::SetNextItemWidth(-1.0F);
        ImGui::DragFloat3("##Point light", &light_position_.x, 0.05F);
        ImGui::TextUnformatted("Base color");
        ImGui::SetNextItemWidth(-1.0F);
        ImGui::ColorEdit3("##Base color", &base_color_.x);
        ImGui::SliderFloat("Shininess", &shininess_, 2.0F, 128.0F);
        ImGui::SliderFloat("UV scale", &uv_scale_, 0.25F, 8.0F);
        ImGui::TextUnformatted("Filtering");
        ImGui::SetNextItemWidth(-1.0F);
        if (ImGui::Combo(
                "##Filtering", &filter_choice_, "Nearest\0Bilinear\0Trilinear mipmaps\0")) {
            const GLenum min_filters[] = {GL_NEAREST, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR};
            const GLenum mag = filter_choice_ == 0 ? GL_NEAREST : GL_LINEAR;
            checker_.set_filter(min_filters[filter_choice_], mag);
        }
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Shadow bonus controls")) {
            if (ImGui::Button("Frame shadow scene")) {
                yaw_ = 0.6F;
                pitch_ = 0.45F;
                radius_ = 5.0F;
            }
            ImGui::Checkbox("Apply shadow", &apply_shadow_);
            ImGui::SliderFloat("Light yaw", &shadow_yaw_, -kPi, kPi);
            ImGui::SliderFloat("Light elevation", &shadow_pitch_, 0.2F, 1.4F);
            ImGui::SliderFloat("Depth bias", &shadow_bias_, 0.0F, 0.02F, "%.4f");
        }
        ImGui::PopItemWidth();
        ImGui::TextWrapped("Drag a view to orbit; wheel to zoom.");
        ImGui::End();
    }

    void ensure_target_size(int index, int width, int height) {
        if (target_sizes_[index] == std::array<int, 2>{width, height}) { return; }
        colors_[index].storage(1, GL_RGBA8, width, height);
        colors_[index].set_filter(GL_LINEAR, GL_LINEAR);
        allocate_depth(depths_[index], width, height);
        framebuffers_[index].attach_texture(GL_COLOR_ATTACHMENT0, colors_[index]);
        framebuffers_[index].attach_texture(GL_DEPTH_ATTACHMENT, depths_[index]);
        if (!framebuffers_[index].complete()) {
            throw std::runtime_error("Shader view framebuffer is incomplete");
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        target_sizes_[index] = {width, height};
    }

    glm::vec3 camera_position() const {
        return radius_
               * glm::vec3(
                   std::sin(yaw_) * std::cos(pitch_), std::sin(pitch_),
                   std::cos(yaw_) * std::cos(pitch_));
    }

    glm::mat4 model_matrix() const {
        glm::mat4 model = glm::rotate(glm::mat4(1.0F), angle_, {0.0F, 1.0F, 0.0F});
        if (nonuniform_scale_) { model = glm::scale(model, {1.0F, 0.8F, 1.3F}); }
        if (model_choice_ == 1) { model = glm::scale(model, glm::vec3(1.2F)); }
        return model;
    }

    glm::vec3 shadow_light_direction() const {
        return {
            std::sin(shadow_yaw_) * std::cos(shadow_pitch_), std::sin(shadow_pitch_),
            std::cos(shadow_yaw_) * std::cos(shadow_pitch_)};
    }

    glm::mat4 light_view_projection() const {
        return glm::ortho(-4.0F, 4.0F, -4.0F, 4.0F, 0.1F, 15.0F)
               * glm::lookAt(shadow_light_direction() * 6.0F, glm::vec3(0.0F), {0, 1, 0});
    }

    void draw_shadow_scene(const Shader &shader, bool materials) {
        // Keep the bunny's lowest point on the same ground under positive scaling.
        glm::mat4 bunny_model = glm::rotate(glm::mat4(1.0F), angle_, {0, 1, 0});
        if (nonuniform_scale_) {
            bunny_model = glm::translate(glm::mat4(1.0F), {0, -0.21F, 0})
                          * glm::scale(bunny_model, {1.0F, 0.8F, 1.3F});
        }
        shader.set_uniform("u_model", bunny_model);
        if (materials) { shader.set_uniform("u_base_color", base_color_); }
        bunny_.draw();
        glm::mat4 ground = glm::translate(glm::mat4(1.0F), {0, -1.05F, 0});
        ground = glm::rotate(ground, -0.5F * kPi, {1, 0, 0});
        ground = glm::scale(ground, glm::vec3(3.0F));
        shader.set_uniform("u_model", ground);
        if (materials) { shader.set_uniform("u_base_color", glm::vec3(0.65F, 0.69F, 0.72F)); }
        plane_.draw();
    }

    void render_shadow_depth() {
        shadow_framebuffer_.bind();
        glViewport(0, 0, kShadowSize, kShadowSize);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glClear(GL_DEPTH_BUFFER_BIT);
        if (depth_shader_.handle() != 0) {
            depth_shader_.use();
            depth_shader_.set_uniform("u_light_view_projection", light_view_projection());
            draw_shadow_scene(depth_shader_, false);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
    }

    void render_view(int index, int width, int height) {
        framebuffers_[index].bind();
        glViewport(0, 0, width, height);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glClearColor(0.10F, 0.13F, 0.17F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        const Shader &shader = shaders_[index];
        if (shader.handle() != 0) {
            const glm::vec3 eye = camera_position();
            shader.use();
            shader.set_uniform("u_view", glm::lookAt(eye, glm::vec3(0.0F), {0, 1, 0}));
            shader.set_uniform(
                "u_projection",
                glm::perspective(
                    glm::radians(45.0F), static_cast<float>(width) / height, 0.1F, 30.0F));
            shader.set_uniform("u_camera_pos", eye);
            shader.set_uniform("u_shininess", shininess_);
            if (index == kShadowView) {
                shader.set_uniform("u_light_view_projection", light_view_projection());
                shader.set_uniform("u_light_direction", shadow_light_direction());
                shader.set_uniform("u_shadow_bias", shadow_bias_);
                shader.set_uniform("u_apply_shadow", apply_shadow_);
                shadow_depth_.bind_unit(2);
                shader.set_uniform("u_shadow_map", 2);
                draw_shadow_scene(shader, true);
            } else {
                shader.set_uniform("u_model", model_matrix());
                shader.set_uniform("u_light_pos", light_position_);
                shader.set_uniform("u_base_color", base_color_);
                shader.set_uniform("u_uv_scale", uv_scale_);
                checker_.bind_unit(0);
                shader.set_uniform("u_texture", 0);
                (model_choice_ == 0 ? bunny_ : plane_).draw();
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
    }

    Mesh bunny_{load_bunny()};
    Mesh plane_{make_plane()};
    GLTexture2D checker_, shadow_depth_;
    GLFramebuffer shadow_framebuffer_;
    Shader depth_shader_;
    std::string depth_error_;
    std::array<Shader, kViewCount> shaders_;
    std::array<std::string, kViewCount> shader_errors_;
    std::array<GLFramebuffer, kViewCount> framebuffers_;
    std::array<GLTexture2D, kViewCount> colors_, depths_;
    std::array<std::array<int, 2>, kViewCount> target_sizes_{};
    glm::vec3 light_position_{2.0F, 3.0F, 3.0F};
    glm::vec3 base_color_{0.72F, 0.72F, 0.93F};
    float yaw_ = 0.6F, pitch_ = 0.18F, radius_ = 4.0F;
    float angle_ = 0.0F, shininess_ = 32.0F, uv_scale_ = 2.0F;
    float shadow_yaw_ = -0.9F, shadow_pitch_ = 0.65F, shadow_bias_ = 0.0015F;
    int model_choice_ = 0, filter_choice_ = 2;
    bool spin_ = false, nonuniform_scale_ = false;
    bool apply_shadow_ = true;
};

Assignment2App::Assignment2App() : impl_(std::make_unique<Impl>()) { }
Assignment2App::~Assignment2App() = default;
void Assignment2App::draw() { impl_->draw(); }
