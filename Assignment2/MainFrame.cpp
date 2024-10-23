#include "MainFrame.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

std::string readShaderFile(const char *filePath) {
    std::ifstream shaderFile;
    shaderFile.open(filePath);
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();
    return shaderStream.str();
}

unsigned int compileShader(const char *filePath, GLenum shaderType) {
    std::string shaderCode = readShaderFile(filePath);
    const char *shaderSource = shaderCode.c_str();

    unsigned int shader;
    shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return shader;
}

unsigned int createShaderProgram(const char *vertexPath, const char *fragmentPath) {
    unsigned int vertexShader = compileShader(vertexPath, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentPath, GL_FRAGMENT_SHADER);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

namespace {

    float scale = 1.f;
    float aspect = 1.f;

#ifdef __APPLE__
    unsigned int SCR_WIDTH = 600;
    unsigned int SCR_HEIGHT = 600;
#else
    unsigned int SCR_WIDTH = 1000;
    unsigned int SCR_HEIGHT = 1000;
#endif

    void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
        scale *= std::pow(1.1f, (float) yoffset);
    }

    void FrameBufferSizeCallback(GLFWwindow *window, int width, int height) {
        SCR_WIDTH = width;
        SCR_HEIGHT = height;
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);        // Set the viewport to cover the new window
        aspect = (float) SCR_WIDTH /
                 (float) SCR_HEIGHT;  // Set the aspect ratio of the clipping area to match the viewport
    }
}

void MainFrame::LeftMouseMove(float start_x, float start_y, float curr_x, float curr_y) {
    glm::mat4x4 transform_mat(1.f);
    glm::vec2 V = glm::vec2(curr_x, curr_y) - glm::vec2(start_x, start_y);
    glm::vec2 A(-V.y, V.x);
    glm::vec3 start = Screen2World(start_x, start_y);
    glm::vec3 start_a = Screen2World(start_x + A.x, start_y + A.y);
    glm::vec3 rot_axis = glm::normalize(start_a - start);
    float k = 0.01f;
    float rot_angle = k * glm::length(A);
    glm::mat4x4 rot_mat = glm::rotate(glm::mat4x4(1.f), rot_angle, rot_axis);
    glm::mat4x4 trans_mat = glm::translate(glm::mat4x4(1.f), mesh_.center_);
    glm::mat4x4 reverse_trans = glm::translate(glm::mat4x4(1.f), -mesh_.center_);
    transform_mat = trans_mat * rot_mat * reverse_trans;

    mesh_.ApplyTransform(transform_mat);
}

void MainFrame::VisualizeWorldSpace() {
}


// -------------------------------------------------------------------------------------
// -------------------------- No need to change ----------------------------------------
// -------------------------------------------------------------------------------------
void MainFrame::MainLoop() {
    // glfw: initialize and configure
    glfwInit();

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation, set viewport with width=1000 and height=1000
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3DModeling", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, FrameBufferSizeCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    // glad: load all OpenGL function pointers
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    const float alpha = 0.3f;
    const float beta = 0.1f;

    const float r = 5.f;
    camera_.LookAt(r * glm::vec3(std::cos(alpha) * std::cos(beta), std::cos(alpha) * std::sin(beta), std::sin(alpha)),
                   glm::vec3(0.f, 0.f, 0.f),
                   glm::vec3(0.f, 0.f, 1.f));
    glEnable(GL_DEPTH_TEST);

    unsigned int shaderProgram = createShaderProgram("../vertex_shader.glsl", "../fragment_shader.glsl");

    unsigned int VBO, VAO, NBO, EBO, TBO;
    mesh_.setupBuffers(VAO, VBO, NBO, EBO, TBO);

    // render loop
    while (!glfwWindowShouldClose(window)) {
        ProcessInput(window);

        // Apply camera projection;
        camera_.Perspective(90.f, aspect, .5f, 10.f);
        camera_.UpdateScale(scale);
        scale = 1.f;
        camera_.ApplyProjection();

        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);        // Clear the display
        glShadeModel(GL_SMOOTH);
        DrawScene(VAO, shaderProgram, mesh_.indices_.size());

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    // glfw: terminate, clearing addl previously allocated GLFW resources.
    glfwTerminate();
}

void MainFrame::ProcessInput(GLFWwindow *window) {
    int current_l_mouse_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

    // Handle left mouse
    if (current_l_mouse_state == GLFW_PRESS) {
        double xposd, yposd;
        float xpos, ypos;
        glfwGetCursorPos(window, &xposd, &yposd);
        xpos = float(xposd);
        ypos = float(SCR_HEIGHT - yposd);
        if (l_mouse_state_ == GLFW_RELEASE) {
            LeftMouseClick(xpos, ypos);
            l_click_cursor_x_ = xpos;
            l_click_cursor_y_ = ypos;
        }
        if (l_mouse_state_ == GLFW_PRESS &&
            (std::abs(xpos - last_cursor_x_) > 2.f || std::abs(ypos - last_cursor_y_) > 2.f)) {
            LeftMouseMove(l_click_cursor_x_, l_click_cursor_y_, xpos, ypos);
        }
        last_cursor_x_ = float(xpos);
        last_cursor_y_ = float(ypos);
    }
    if (current_l_mouse_state == GLFW_RELEASE) {
        if (l_mouse_state_ == GLFW_PRESS) {
            LeftMouseRelease();
        }
    }
    l_mouse_state_ = current_l_mouse_state;

    // Handle right mouse
    int current_r_mouse_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    if (current_r_mouse_state == GLFW_PRESS) {
        double xposd, yposd;
        float xpos, ypos;
        glfwGetCursorPos(window, &xposd, &yposd);
        xpos = float(xposd);
        ypos = float(SCR_HEIGHT - yposd);
        if (r_mouse_state_ == GLFW_RELEASE) {
            RightMouseClick(xpos, ypos);
        }
        if (r_mouse_state_ == GLFW_PRESS &&
            (std::abs(xpos - last_cursor_x_) > 2.f || std::abs(ypos - last_cursor_y_) > 2.f)) {
            RightMouseMove(last_cursor_x_, last_cursor_y_, xpos, ypos);
        }
        last_cursor_x_ = float(xpos);
        last_cursor_y_ = float(ypos);
    }
    if (current_r_mouse_state == GLFW_RELEASE) {
        if (r_mouse_state_ == GLFW_PRESS) {
            RightMouseRelease();
        }
    }
    r_mouse_state_ = current_r_mouse_state;
}

void MainFrame::LeftMouseClick(float x, float y) {
}

void MainFrame::LeftMouseRelease() {
}

void MainFrame::RightMouseClick(float x, float y) {
    return;
}

void MainFrame::RightMouseMove(float start_x, float start_y, float curr_x, float curr_y) {
    glm::vec2 s_start(start_x, start_y);
    glm::vec2 s_cur(curr_x, curr_y);
    glm::vec2 V = s_cur - s_start;
    glm::vec2 A = glm::vec2(-V.y, V.x);
    glm::vec3 rot_axis = glm::normalize(Screen2World(A + s_start) - Screen2World(s_start));
    glm::mat4x4 rot_mat = glm::rotate(glm::mat4x4(1.f), 0.007f * glm::length(A), rot_axis);
    camera_.ApplyTransform(rot_mat);
}

void MainFrame::RightMouseRelease() {
    return;
}

glm::vec3 MainFrame::Camera2World(const glm::vec3 &x, float w) {
    return glm::vec3(camera_.view_mat_inv_ * glm::vec4(x, w));
}

glm::vec3 MainFrame::World2Camera(const glm::vec3 &x, float w) {
    return glm::vec3(camera_.view_mat_ * glm::vec4(x, w));
}

glm::vec3 MainFrame::Screen2World(const glm::vec2 &v, float depth) {
    float x = v.x / SCR_WIDTH * 2.f - 1.f;
    float y = v.y / SCR_HEIGHT * 2.f - 1.f;
    float focal = std::tan(camera_.fov_ * .5f / 180.f * glm::pi<float>());
    glm::vec4 v_camera(x * focal * aspect, y * focal, -1.f, 1.f);
    v_camera = v_camera * depth;
    glm::vec4 v_world = camera_.view_mat_inv_ * v_camera;
    return glm::vec3(v_world);
}

glm::vec3 MainFrame::Screen2World(float scr_x, float scr_y, float camera_z) {
    float x = scr_x / SCR_WIDTH * 2.f - 1.f;
    float y = scr_y / SCR_HEIGHT * 2.f - 1.f;
    float focal = std::tan(camera_.fov_ * .5f / 180.f * glm::pi<float>());
    glm::vec4 v_camera(x * focal * aspect, y * focal, -1.f, 1.f);
    v_camera = v_camera * -camera_z;
    glm::vec4 v_world = camera_.view_mat_inv_ * v_camera;
    return glm::vec3(v_world);
}

void MainFrame::DrawScene(unsigned int VAO, unsigned int shaderProgram, size_t indexCount) {
    // Draw mesh
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);

    glm::mat4 view = camera_.view_mat_;
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glm::mat4 projection = camera_.projection_mat_;
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, glm::value_ptr(lightPos));
    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(camera_.eye_));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(glm::vec3(1.0f, 0.5f, 0.31f)));


    mesh_.Draw(VAO, shaderProgram, indexCount);

    VisualizeWorldSpace();
}