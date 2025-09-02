#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int mode = 1;


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        mode = 1;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        mode = 2;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        mode = 3;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
        mode = 4;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);  // Set the viewport to cover the new window

    // Set the aspect ratio of the clipping area to match the viewport
    float expected_ratio = (float) SCR_WIDTH / (float) SCR_HEIGHT;
    float ratio = (float)width / (float)height;
    float trans_ratio = expected_ratio / ratio;

    glMatrixMode(GL_PROJECTION);      // To operate on the Projection matrix
    glLoadIdentity();                 // Reset the Projection matrix

    if (trans_ratio > 1.0) {
        glOrtho(-1.0, 1.0, -1.0 * trans_ratio, 1.0 * trans_ratio, -1, 1);
    }
    else {
        glOrtho(-1.0 / trans_ratio, 1.0 / trans_ratio, -1.0, 1.0, -1, 1);
    }
}

int main()
{
    // glfw: initialize and configure
    glfwInit();

    // glfw window creation, set viewport with width=800 and height=600
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL_Demo", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glClearColor(0.0, 0.0, 0.0, 1.0);    
        glClear(GL_COLOR_BUFFER_BIT);        // Clear the display

        if (mode == 1) {
            glBegin(GL_TRIANGLES);           // Draw filled triangle
            glColor3f(1.f, 0.f, 0.f);        // Specify color for each vertex of triangle
            glVertex3f(-0.3f, -0.3f, 0.f);   // Specify position for each vertex of triangle
            glColor3f(0.f, 1.f, 0.f);
            glVertex3f(0.3f, -0.3f, 0.f);
            glColor3f(0.f, 0.f, 1.f);
            glVertex3f(0.f, 0.3f, 0.f);
            glEnd();                         // OpenGL draws the filled triangle
        }
        if (mode == 2) {
            glPointSize(5.0);
            glBegin(GL_POINTS);
            glVertex2f(-0.3f, -0.3f);
            glVertex2f(0.0f, 0.3f);
            glVertex2f(0.3f, -0.3f);
            glEnd();
        }
        if (mode == 3) {
            glLineWidth(5.0);
            glBegin(GL_LINE_LOOP);
            glVertex2f(-0.3f, -0.3f);
            glVertex2f(0.0f, 0.3f);
            glVertex2f(0.3f, -0.3f);
            glEnd();
        }
        if (mode == 4) {
            glBegin(GL_POLYGON);
            glColor3f(1.f, 0.f, 0.f);
            glVertex3f(-0.3, -0.3f, 0.f);
            glColor3f(0.f, 1.f, 0.f);
            glVertex3f(-0.3, 0.3f, 0.f);
            glColor3f(0.f, 0.f, 1.f);
            glVertex3f(0.3f, 0.3f, 0.f);
            glColor3f(1.f, 1.f, 1.f);
            glVertex3f(0.3, -0.3f, 0.f);
            glEnd();
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    // glfw: terminate, clearing addl previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}


