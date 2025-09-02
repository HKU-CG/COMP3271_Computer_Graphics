# Programming Assignment 1
## Due date: TBD

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

TBD