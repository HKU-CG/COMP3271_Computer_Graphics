#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "code.h"
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <vector>


//////////////////////////////////////////////////////
// The Global Variables To Be Used

struct Triangle {
    double vertices[3][2];
    double matrix[3][3];
    int color_index;
};

// list of triangles, each element is of type Triangle
// to access the kth triangle, just use triangles[k]
// to get the number of triangles in the list, use triangles.size()
std::vector<Triangle> triangles;

// a triangle object for temporary storage of points
Triangle triangle_to_draw;
// count the number of points specified in this triangle
int point_count = 0;

// color array for triangles
// size is 11. So color_index should range from 0 to 10 for triangles.
double color_array[][3] = {
    {0.9,0,0}, //red
    {0,0.5,0.4},
    {0.1,0.2,0.46},
    {0.9,0.9,0},
    {0,1.0,0},
    {0,1.0,1.0},
    {0,0,1.0},
    {1.0,0,1.0},
    {0.9,0.6,0},
    {0.9,1.0,0.6},
    {0.2,0.2,0.2}
};


/////////////////////////////////////////////////////////////////////////////////////////
// This function is called to clear triangles.

void ClearTriangles() {
    triangles.clear();
    point_count = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// This function is called to draw triangles in the Triangles window.

void DrawTriangles() {
    // uncomment this line if you would like to unfill triangles
    // glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

    // sample code to draw vertices of triangle
    glColor3d(1.0, 1.0, 1.0);
    glPointSize(4);
    glBegin(GL_POINTS);
    for (int i = 0; i < point_count; i++) {
        glVertex2d(triangle_to_draw.vertices[i][0], triangle_to_draw.vertices[i][1]);
    }
    glEnd();

    // TODO: Add code to draw triangles here. Use triangles.size() to get number of triangles.
    // Use triangles[i] to get the ith triangle.
    // Remember to set current gl color from the color_array.
    // If the number of triangles exceeds the length of the color list, you can start iterating the color list from the beginning again.
}


/////////////////////////////////////////////////////////////////////////////////////////
// This function is called to handle mouse left click events.
// m_x and m_y are the x and y coordinates of the clicked point in OpenGL coordinate system.

void MouseInteraction(double m_x, double m_y) {
    // TODO: Store the point temporarily into the variable triangle_to_draw.
    // When 3 points are specified, we get a new triangle.
    // Store the points of the triangle into a new element of the list 'triangles'
    // Use push_back function from std::Vector to add new triangles to the list
}
