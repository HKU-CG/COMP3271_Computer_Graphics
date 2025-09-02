#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <chrono>
#include <iostream>
#include <vector>

#include "code.h"

// settings
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;

// states
double gl_frame_bound_x = 1;
double gl_frame_bound_y = 1;

int last_mouse_state = 0;

double coords_sample_1[4][3][2] = {
    {{-0.5, -0.5}, {0, 0.5}, {0.5, -0.5}},
    {{-0.5, -0.5}, {-0.25, 0}, {0, -0.5}},
    {{-0.25, 0}, {0, 0.5}, {0.25, 0}},
    {{0, -0.5}, {0.25, 0}, {0.5,-0.5}}};

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void ProcessInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
        ClearTriangles();
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) { // Test Sierpinski Gasket
        ClearTriangles();
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 3; j++) {
                MouseInteraction(coords_sample_1[i][j][0], coords_sample_1[i][j][1]);
            }
        }
    }
    int current_mouse_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if (current_mouse_state == GLFW_RELEASE && last_mouse_state == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        double x = (xpos / SCR_WIDTH - 0.5) * gl_frame_bound_x * 2.0;
        double y = -(ypos / SCR_HEIGHT - 0.5) * gl_frame_bound_y * 2.0;

        // Call mouse interaction.
        MouseInteraction(x, y);
    }
    last_mouse_state = current_mouse_state;
}

int main()
{
    // glfw: initialize and configure
    if (!glfwInit()) {
        std::cout << "GLFW initializeion failed." << std::endl;
    }

    // glfw window creation, set viewport with width=800 and height=600
    GLFWwindow* window_0 = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Triangles", NULL, NULL);
    glfwMakeContextCurrent(window_0);

    // glad: load all OpenGL function pointers
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // render loop
    while (!glfwWindowShouldClose(window_0)) {
        // window 0
        glfwMakeContextCurrent(window_0);
        ProcessInput(window_0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glClearColor(0.0, 0.0, 0.0, 1.0);    
        glClear(GL_COLOR_BUFFER_BIT);        // Clear the display

        DrawTriangles();
        
        glfwSwapBuffers(window_0);

        
        glfwPollEvents();
    }

    // glfw: terminate, clearing addl previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}
