# Programming Assignment 1
## Due date: 23:59, Oct. 13, 2025 HKT

## Part 1

### Goal
2D Drawing in OpenGL.

### Objectives
- You are required to deal with the mouse interaction and collect these points, which are input by
the mouse to form triangles.
- Draw these triangles with the OpenGL function.

### Code Template
- You will be offered a template, so you don’t need to do more work to configure the template.
- The template has already implemented some initial work, such as initializing one window, configuring
OpenGL and so on.
- The interfaces of the functions that you need to fill in and some data structures you will use are given in
‘code.h’ and ‘code.cpp’.
- Feel free to use your own environment and library. But you should schedule a meeting and demo your
  program to the TA.

### Usage
#### Linux/Macos
```shell
$ cd COMP3271_Computer_Graphics/Assignment1/Part1
$ mkdir build
$ cd build
$ cmake ..
$ make
$ ./Main
```
#### Windows (PowerShell)
```shell
$ cd COMP3271_Computer_Graphics/Assignment1/Part1
$ mkdir build
$ cd build
$ cmake ..
```
Then open PA1.sln under `build` folder, set Main as Startup Projects.

### Task
Your task is to implement the following two functions whose interfaces are given in code.cpp.

```c++
void MouseInteraction(double x, double y);
```

```c++
void DrawTriangles();
```

### Requirements

One window should be opened to display triangles. Interactive techniques are provided with related
functions. You just need to fill in four functions to make the whole thing work.

## Part 2

### Goal

This assignment is about mouse-controlled 3D model editing in OpenGL.

### Objectives

- In this assignment, only the implementation of mouse interaction control is required. You
  should implement a few event handlers to handle model transformation (rotation, translation and
  extrusion).

### Code Template

A project example is prepared for you to help you focus on the interface design.
  The functions of the template include:

- Simple 3D primitive rendering (done).
- Face subdivision (done).
- **Navigate the control mode by switching the number keys for rotation, translation, face**
  **subdivision and extrusion (todo).**
- **Viewpoint rotation, translation and scaling implementation (todo).**

### Usage

#### Linux/Macos

```shell
$ cd COMP3271_Computer_Graphics/Assignment1/Part2
$ mkdir build
$ cd build
$ cmake ..
$ make
$ ./Main
```

#### Windows (PowerShell)

```shell
$ cd COMP3271_Computer_Graphics/Assignment1/Part2
$ mkdir build
$ cd build
$ cmake ..
```

Then open GraphicsPA2.sln under `build` folder, set Main as Startup Projects.

### Task

Your task is to implement the following function where the **4** "TODO"s are marked in the 'MainFrame.cpp' file.

```c++
void LeftMouseMode(float startX, float startY, float currentX, float currentY);
```

This function is called by the template whenever the mouse is moved. The start_X and curr_Y are the
screen coordinates of the mouse position when it starts moving, and start_Y and curr_Y are the current
screen coordinates of the mouse position.

### Requirements

One window should be opened to show the required interactive techniques with related functions. See the
demo video for reference.

Necessary comments to explain your code is required.

**Submit your work (code.cpp and MainFrame.cpp) on Moodle before the deadline.**