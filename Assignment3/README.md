# Programming Assignment 3

## Due date: 23:59, Dec. 3, 2024 HKT

### Goal

Path Tracing Renderer.

### Objectives

You are required to implement

- the ray hit test function for the sphere and the triangle,
- the random hemisphere sampling function,
- the diffuse reflection function.

### Code Template

- You will be offered a template, so you don’t need to do more work to configure the template.
- By default, the template outputs an image of the rendered scene in the `outputs` folder.
- Feel free to use your own environment and library. But you should schedule a meeting and demo your
  program to the TA.

### Usage

#### Linux/Macos

```shell
$ cd COMP3271_Computer_Graphics/Assignment3
$ mkdir build
$ cd build
$ cmake ..
$ make
$ ./Main
```

#### Windows (PowerShell)

```shell
$ cd COMP3271_Computer_Graphics/Assignment3
$ mkdir build
$ cd build
$ cmake ..
```

Then open Assignment3.sln under `build` folder, set Main as Startup Projects.

### Task

Your task is to implement the following functions whose interfaces are given in `Hittable.cpp`.

```cpp
bool Sphere::Hit(const Ray &ray, HitRecord *hit_record)
```

```cpp
bool Triangle::Hit(const Ray &ray, HitRecord *hit_record)
```

and the following functions whose interfaces are given in `main.cpp`.

```cpp
Color Shade(const Hittable &hittable_collection,
            const HitRecord &hit_record,
            int trace_depth)
```

```cpp
Vec SampleHemisphere(const Vec &normal)
```

#### bonus task

- Implement a mirror reflection model in Shade function in the following method:

```cpp
Color Shade(const Hittable &hittable_collection,
            const HitRecord &hit_record,
            int trace_depth)
```

Refer to the tutorial slides for more details. Hint for the implementation: we can define when the shininess is
negative, the material is a perfect mirror reflection.

### Requirements

Necessary comments to explain your code is required.

**Submit your work `main.cpp` and `Hittable.cpp` (If other files are modified, please also submit them) on Moodle before
the deadline.**