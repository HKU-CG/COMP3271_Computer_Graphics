#pragma once
#include "Common.h"
#include "Mesh.h"
#include "Camera.h"

// -----------------------------------------------------------------------------
// Editing mode definition
// -----------------------------------------------------------------------------
#define OBJ_ROTATION 1                          // object rotation
#define OBJ_TRANSLATION 2                       // object translation
#define OBJ_SUBDIVIDE 3                         // object face subdivision
#define OBJ_EXTRUDE 4                           // object face extrusion

// -----------------------------------------------------------------------------
// MainFrame: The class controlling the overall rendering & interaction logic
// -----------------------------------------------------------------------------
class MainFrame {
public:
    // ---------------
    // Main functions
    // ---------------
    MainFrame() = default;
    void MainLoop();

    // Function to handle all user inputs
    void ProcessInput(GLFWwindow* window);

    // This function is called once left mouse clicked.
    // Params:
    //     x: the x value at screen coordinate system.
    //     y: the y value at screen coordinate system.
    void LeftMouseClick(float x, float y);

    // This function is called when left mouse moving.
    // Params:
    //     start_x: the x value of the start cursor position at screen coordinate system.
    //     start_y: the y value of the start cursor position at screen coordinate system.
    //     end_x: the x value of the end(current) cursor position at screen coordinate system.
    //     end_y: the y value of the end(current) cursor position at screen coordinate system.
    void LeftMouseMove(float start_x, float start_y, float end_x, float end_y);

    // This function is called once left mouse released.
    void LeftMouseRelease();

    // Similar as above.
    void RightMouseClick(float x, float y);
    void RightMouseMove(float start_x, float start_y, float end_x, float end_y);
    void RightMouseRelease();

    // Functions to render the scene.
    void DrawScene(unsigned int VAO, unsigned int shaderProgram, size_t indexCount);
    void VisualizeWorldSpace();

    // ---------------------------
    // Some other useful functions
    // ---------------------------

    // Transform the coordinate from camera/eye space to world space.
    // Params:
    //     x: the 3d vector/point in the camera space.
    //     w: the value of the extra dimension for homography coordinates.
    // Return:
    //     the 3d vector/point in the world space.
    glm::vec3 Camera2World(const glm::vec3& x, float w = 1.f);

    // Transform the coordinate from world space to camera/eye space.
    // Params:
    //     x: the 3d vector/point in the world space.
    //     w: the value of the extra dimension for homography coordinates.
    // Return:
    //     the 3d vector/point in the camera space.
    glm::vec3 World2Camera(const glm::vec3& x, float w = 1.f);

    // Transform the coordinate from screen space to world space.
    // Params:
    //     scr_x: the x value in the screen space.
    //     scr_y: the y value in the screen space.
    //     camera_z: expected z value in the *camera* space.
    // Return:
    //     the 3d vector/point in the world space.
    glm::vec3 Screen2World(float scr_x, float scr_y, float camera_z = -1.f);

    // Another way to transform the coordinate from screen space to world space.
    // Params:
    //     v: the 2d point position in the screen space.
    //     depth: expected *depth* value in the camera space. ***depth = -camera_z***
    // Return:
    //     the 3d vector/point in the world space.
    glm::vec3 Screen2World(const glm::vec2& v, float depth = 1.f);

    // Members
    Mesh mesh_;                                                                      // mesh_ keeps the geometry information of the object
    Camera camera_;                                                                  // camera_ keeps the intrinsic/extrinsic camera parameters

    // Some states
    int l_mouse_state_ = GLFW_RELEASE;
    int r_mouse_state_ = GLFW_RELEASE;

    int modeling_state_ = OBJ_ROTATION;

    float last_cursor_x_ = 0;
    float last_cursor_y_ = 0;
    float l_click_cursor_x_ = 0;
    float l_click_cursor_y_ = 0;
};
