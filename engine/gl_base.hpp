#pragma once

#include <map>
#include <utility>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdexcept>
// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <spdlog/spdlog.h>

// CRTP
template <typename Derived>
class GLObject {
  public:
    GLObject() { static_cast<Derived *>(this)->create(); }
    GLObject(const GLObject &) = delete;
    GLObject &operator=(const GLObject &) = delete;
    GLObject(GLObject &&other) noexcept : handle_(std::exchange(other.handle_, 0)) { }

    GLObject &operator=(GLObject &&other) noexcept {
        if (this != &other) {
            destroy();
            handle_ = std::exchange(other.handle_, 0);
        }
        return *this;
    }

    ~GLObject() { destroy(); }

    operator unsigned int() const { return handle_; }

    unsigned int handle_ = 0;

  private:
    void destroy() {
        if (handle_ != 0) {
            static_cast<Derived *>(this)->destroy();
            handle_ = 0;
        }
    }
};

// GLBuffer in OpenGL3.3
class GLBuffer : public GLObject<GLBuffer> {
  public:
    void create() { glGenBuffers(1, &handle_); }
    void destroy() { glDeleteBuffers(1, &handle_); }

    void bind(GLenum target) const { glBindBuffer(target, handle_); }
    static void unbind(GLenum target) { glBindBuffer(target, 0); }

    void set_data(
        size_t size, const void *data, GLenum usage, GLenum target = GL_ARRAY_BUFFER) {
        bind(target);
        glBufferData(target, size, data, usage);
        unbind(target);
    }

    void set_subdata(
        GLintptr offset, size_t size, const void *data, GLenum target = GL_ARRAY_BUFFER) {
        bind(target);
        glBufferSubData(target, offset, size, data);
        unbind(target);
    }

    void bind_base(GLenum target, unsigned int index) const {
        glBindBufferBase(target, index, handle_);
    }
};

class GLVertexArray : public GLObject<GLVertexArray> {
  public:
    void create() { glGenVertexArrays(1, &handle_); }

    void destroy() { glDeleteVertexArrays(1, &handle_); }

    void bind() const { glBindVertexArray(handle_); }

    static void unbind() { glBindVertexArray(0); }

    // attrib data must come from a buffer currently bound to GL_ARRAY_BUFFER
    void set_attrib(
        GLuint attribIndex, GLint size, GLenum type, GLboolean normalized, GLsizei stride,
        const void *offset) {
        bind();
        glEnableVertexAttribArray(attribIndex);
        glVertexAttribPointer(attribIndex, size, type, normalized, stride, offset);
    }

    void set_attrib_divisor(GLuint attribIndex, GLuint divisor) {
        bind();
        glVertexAttribDivisor(attribIndex, divisor);
    }

    void set_element_buffer(GLuint buffer) {
        bind();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    }
};

class GLTexture2D : public GLObject<GLTexture2D> {
  public:
    void create() { glGenTextures(1, &handle_); }

    void destroy() { glDeleteTextures(1, &handle_); }

    void bind() const { glBindTexture(GL_TEXTURE_2D, handle_); }

    static void unbind() { glBindTexture(GL_TEXTURE_2D, 0); }

    void storage(GLsizei levels, GLenum internalFormat, GLsizei w, GLsizei h) {
        bind();
        for (GLsizei level = 0; level < levels; level++) {
            glTexImage2D(
                GL_TEXTURE_2D, level, internalFormat, std::max(1, w >> level),
                std::max(1, h >> level), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        }
        unbind();
    }

    void sub_image(
        GLint level, GLint x, GLint y, GLsizei w, GLsizei h, GLenum format, GLenum type,
        const void *data) {
        bind();
        glTexSubImage2D(GL_TEXTURE_2D, level, x, y, w, h, format, type, data);
        unbind();
    }

    void set_filter(GLenum min, GLenum mag) {
        bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
        unbind();
    }

    void bind_unit(GLuint unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, handle_);
    }
};

class GLFramebuffer : public GLObject<GLFramebuffer> {
  public:
    void create() { glGenFramebuffers(1, &handle_); }

    void destroy() { glDeleteFramebuffers(1, &handle_); }

    void attach_texture(GLenum attachment, GLuint tex, GLint level = 0) {
        bind();
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, tex, level);
    }

    bool complete() const {
        GLint previous;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previous);
        bind();
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        glBindFramebuffer(GL_FRAMEBUFFER, previous);
        return status == GL_FRAMEBUFFER_COMPLETE;
    }

    void bind(GLenum target = GL_FRAMEBUFFER) const { glBindFramebuffer(target, handle_); }
};

struct GLFWContext {
    GLFWContext() {
        if (!glfwInit()) { throw std::runtime_error("Failed to init glfw"); }
    }
    ~GLFWContext() { glfwTerminate(); }

    GLFWContext(const GLFWContext &) = delete;
    GLFWContext &operator=(const GLFWContext &) = delete;
    GLFWContext(GLFWContext &&) = delete;
    GLFWContext &operator=(GLFWContext &&) = delete;
};

class GLWindow {
  private:
    GLFWwindow *window_ = nullptr;

  public:
    GLWindow(int width, int height, const std::string &title) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef PLATFORM_MACOSX
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
        window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (window_ == nullptr) { throw std::runtime_error("Failed to create GLFW window"); }
        glfwMakeContextCurrent(window_);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            glfwTerminate();
            throw std::runtime_error("GLAD failed to load GL functions");
        }

        spdlog::info("OpenGL version: {}", (const char *)glGetString(GL_VERSION));
    }

    ~GLWindow() {
        if (window_) {
            glfwDestroyWindow(window_);
            window_ = nullptr;
        }
    }

    GLWindow(const GLWindow &) = delete;
    GLWindow &operator=(const GLWindow &) = delete;
    GLWindow(const GLWindow &&) = delete;
    GLWindow &operator=(const GLWindow &&) = delete;

    [[nodiscard]] bool should_close() const { return glfwWindowShouldClose(window_); }
    void swap_buffers() { glfwSwapBuffers(window_); }

    void poll_events() { glfwPollEvents(); }

    void get_framebuffer_size(int &w, int &h) const { glfwGetFramebufferSize(window_, &w, &h); }

    operator GLFWwindow *() const { return window_; }
};

class Shader {
  public:
    Shader() = default;
    explicit Shader(const std::string &vertex_path, const std::string &fragment_path) {
        const std::string vsrc = read_shader_file(vertex_path);
        const std::string fsrc = read_shader_file(fragment_path);

        const unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        const unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        try {
            compile_shader(vertex_shader, vsrc);
            compile_shader(fragment_shader, fsrc);

            id_ = glCreateProgram();
            glAttachShader(id_, vertex_shader);
            glAttachShader(id_, fragment_shader);
            link_program(id_);
        } catch (...) {
            glDeleteShader(vertex_shader);
            glDeleteShader(fragment_shader);
            release();
            throw;
        }
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
    }

    ~Shader() { release(); }
    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;
    Shader(Shader &&other) noexcept : id_(std::exchange(other.id_, 0)) { }
    Shader &operator=(Shader &&other) noexcept {
        if (this != &other) {
            release();
            id_ = std::exchange(other.id_, 0);
        }
        return *this;
    }

    void use() const { glUseProgram(id_); }

    template <typename... Args>
    void set_uniform(const std::string &name, Args... args) const {
        using T = std::tuple_element_t<0, std::tuple<Args...>>;
        static_assert(
            std::is_same_v<T, int> || std::is_same_v<T, float> || std::is_same_v<T, bool>,
            "set_uniform arguments must be int, float or bool");
        static_assert((std::is_same_v<T, Args> && ...), "set_uniform arguments must be consistent");
        static_assert(
            sizeof...(Args) >= 1 && sizeof...(Args) <= 4,
            "set_uniform only support 1 to 4 scalar arguments");

        int location = glGetUniformLocation(id_, name.c_str());
        if constexpr (std::is_same_v<T, bool>) {
            set_uniform_bool_impl(location, static_cast<int>(args)...);
        } else if constexpr (std::is_same_v<T, int>) {
            set_uniform_int_impl(location, args...);
        } else if constexpr (std::is_same_v<T, float>) {
            set_uniform_float_impl(location, args...);
        }
    }
    void set_uniform(const std::string &name, const glm::vec3 &vec) const {
        int location = glGetUniformLocation(id_, name.c_str());
        set_uniform_float_impl(location, vec.x, vec.y, vec.z);
    }

    void set_uniform(const std::string &name, const glm::mat3 &mat) const {
        int location = glGetUniformLocation(id_, name.c_str());
        use();
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(mat));
    }

    void set_uniform(const std::string &name, const glm::mat4 &mat) const {
        int location = glGetUniformLocation(id_, name.c_str());
        use();
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
    }

    std::string read_shader_file(const std::string &path) {
        std::ifstream file;
        file.exceptions(std::ios::failbit | std::ios::badbit);

        try {
            file.open(path);
            std::stringstream ss;
            ss << file.rdbuf();
            return ss.str();
        } catch (const std::ios_base::failure &e) {
            spdlog::error("Failed to read shader file '{}': {}", path, e.what());
            throw;
        }
    }

    [[nodiscard]] unsigned int handle() const noexcept { return id_; }
    operator unsigned int() const noexcept { return id_; }

  private:
    unsigned int id_ = 0;

    void release() noexcept {
        if (id_ != 0) {
            glDeleteProgram(id_);
            id_ = 0;
        }
    }

    void link_program(unsigned int program) {
        glLinkProgram(program);
        int status = GL_TRUE;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (status != GL_TRUE) [[unlikely]] {
            GLsizei log_length;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
            std::string log;
            log.resize(log_length);
            glGetProgramInfoLog(program, log_length, &log_length, log.data());
            spdlog::error("OpenGL ERROR linking program:\n{}", log);
            throw std::runtime_error("OpenGL program link failed: " + log);
        }
    }

    void compile_shader(unsigned int shader, const std::string &src) {
        const char *srcList[1] = {src.c_str()};
        int srcLenList[1] = {(int)src.size()};
        glShaderSource(shader, 1, srcList, srcLenList);
        glCompileShader(shader);
        int status = GL_TRUE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

        if (status != GL_TRUE) [[unlikely]] {
            GLsizei logLength;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
            std::string log;
            log.resize(logLength);
            glGetShaderInfoLog(shader, logLength, &logLength, log.data());
            log.resize(logLength);
            std::map<int, std::string> lines;
            for (size_t i = 0; i < log.size(); i++) {
                if (i >= 1 && log[i] == '(' && log[i - 1] == '0'
                    && (i == 1 || log[i - 2] == '\n')) {
                    size_t j = log.find_first_of(")\n", i + 1);
                    if (j != log.npos) [[likely]] {
                        size_t k = log.find_first_not_of(") :", j);
                        auto &s = lines[std::atoi(log.substr(i + 1, j - i - 1).c_str())];
                        size_t l = log.find('\n', k);
                        s.append(">>>>> ");
                        s.append(log.substr(k, l == log.npos ? log.npos : l - k));
                        s.push_back('\n');
                    }
                }
            }
            int line = 2;
            std::ostringstream oss;
            auto flg = oss.flags();
            std::string lastErr;
            auto it = lines.find(1);
            if (it != lines.end()) {
                oss << "\033[33;1m  1 |\033[0m ";
                lastErr = it->second;
            } else {
                oss << "\033[37m  1 |\033[0m ";
            }
            for (auto const &c : src) {
                oss << c;
                if (c == '\n') {
                    if (!lastErr.empty()) {
                        oss << "\033[33m" << lastErr << "\033[0m";
                        lastErr.clear();
                    }
                    auto it = lines.find(line);
                    if (it != lines.end()) {
                        oss << "\033[33;1m";
                        lastErr = it->second;
                    } else {
                        oss << "\033[37m";
                    }
                    oss << std::setw(3) << std::right << line;
                    oss << " |\033[0m ";
                    oss.flags(flg);
                    ++line;
                }
            }
            if (!lastErr.empty()) { oss << "\033[33m" << lastErr << "\033[0m"; }
            spdlog::error("OpenGL ERROR compiling shader:\n{}", oss.str());
            if (lines.empty()) { spdlog::error("Log messages:\n{}", log); }
            throw std::runtime_error("OpenGL shader compilation failed: " + log);
        }
    }

    template <typename... Args>
    void set_uniform_bool_impl(int loc, Args... args) const {
        set_uniform_int_impl(loc, args...);
    }

    template <typename... Args>
    void set_uniform_int_impl(int loc, Args... args) const {
        use();
        if constexpr (sizeof...(Args) == 1) {
            glUniform1i(loc, args...);
        } else if constexpr (sizeof...(Args) == 2) {
            glUniform2i(loc, args...);
        } else if constexpr (sizeof...(Args) == 3) {
            glUniform3i(loc, args...);
        } else if constexpr (sizeof...(Args) == 4) {
            glUniform4i(loc, args...);
        }
    }

    template <typename... Args>
    void set_uniform_float_impl(int loc, Args... args) const {
        use();
        if constexpr (sizeof...(Args) == 1) {
            glUniform1f(loc, args...);
        } else if constexpr (sizeof...(Args) == 2) {
            glUniform2f(loc, args...);
        } else if constexpr (sizeof...(Args) == 3) {
            glUniform3f(loc, args...);
        } else if constexpr (sizeof...(Args) == 4) {
            glUniform4f(loc, args...);
        }
    }
};
