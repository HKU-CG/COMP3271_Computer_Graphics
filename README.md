# COMP3271 Computer Graphics

Welcome to the COMP3271 in year 2024-2025!

Here is the code repository of HKU course COMP3271. Any commit and issue will be welcome.

Instructor: [Prof. Taku Komura](https://www.cs.hku.hk/index.php/people/academic-staff/taku)

TA: @[Huancheng Lin](https://hku-cg.github.io/author/huancheng-lin/)

## Instruction

### Environment Setup

#### Macos
- Install Homebrew using the following commands
```shell
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

- Install cmake, make, git, glfw
```shell
brew install cmake make glfw git
```
#### Linux

- Use your package manager to install cmake, build-essential(This is a meta package that contains the tools we need for compiling cxx projects)
```shell
sudo apt install cmake build-essential
```

#### Windows

- Download and install [Visual Studio Community](https://visualstudio.microsoft.com), "Desktop development with c++" should also be installed.
- Download and install [Git](https://git-scm.com) and [Cmake](https://cmake.org)
- Alternatively git and cmake can be installed with winget
```shell
winget install Git.Git
winget install cmake
```

### Compile and run the code

- Get the latest version of this repo
```shell
git clone https://github.com/HKU-CG/COMP3271_Computer_Graphics.git
```
- Compile and run
#### Linux/Macos
```shell
cd COMP3271_Computer_Graphics/<AssignmentFolder/PartX>
mkdir build
cd build
cmake ..
make
./<project_name>
```
#### Windows (PowerShell)
```shell
cd COMP3271_Computer_Graphics/<AssignmentFolder/PartX>
mkdir build
cd build
cmake ..
```
Then open the `.sln` file under `build` folder using visual studio, set `<ProjectName>` as Startup Projects.

Replace `<>` with the actual folder/name.
- Don't hesitate to seek helps with issue workspace

