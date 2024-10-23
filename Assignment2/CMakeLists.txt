cmake_minimum_required (VERSION 3.8)
project(Assignment2)
set(CMAKE_CXX_STANDARD 14)

# GLFW
if (APPLE)
    # include_directories(thirdparty/apple/glfw/include)
    # link_directories(thirdparty/apple/glfw/lib-macos)
    include_directories(/System/Library/Frameworks)
    find_package(glfw3 3.3 REQUIRED)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(OpenGL_LIBRARY OpenGL)
    find_library(IOKit_LIBRARY IOKit)
    find_library(CoreVideo_LIBRARY CoreVideo)
elseif (UNIX)
    include_directories(thirdparty/linux/glfw/include)
    link_directories(thirdparty/linux/glfw/lib)
elseif (WIN32)
    include_directories(thirdparty/windows/glfw/include)
    link_directories(thirdparty/windows/glfw/lib-vc2019)
else()
    message(FATAL_ERROR "Unrecognized platform")
endif()

# GLAD
include_directories(thirdparty/glad/include)
set(SOURCE_FILES thirdparty/glad/src/glad.c MainFrame.cpp MainFrame.h)

# GLM
include_directories(thirdparty/glm)

add_executable(Main ${SOURCE_FILES} Main.cpp Mesh.cpp Camera.cpp)

if (APPLE)
    target_link_libraries(Main glfw ${COCOA_LIBRARY} ${OpenGL_LIBRARY} ${IOKit_LIBRARY} ${CoreVideo_LIBRARY})
elseif (UNIX)
    target_link_libraries(Main glfw3 dl X11 pthread)
else()
    target_link_libraries(Main glfw3)
endif()
