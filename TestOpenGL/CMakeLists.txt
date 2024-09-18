cmake_minimum_required (VERSION 3.8)

project(TestOpenGL)

# GLFW
if (APPLE)
    message("APPLE")
    # The 
    # include_directories(thirdparty/apple/glfw/include)
    # link_directories(thirdparty/apple/glfw/lib-macos)
    include_directories(/System/Library/Frameworks)
    find_package(glfw3 3.3 REQUIRED)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(OpenGL_LIBRARY OpenGL)
    find_library(IOKit_LIBRARY IOKit)
    find_library(CoreVideo_LIBRARY CoreVideo)
elseif (UNIX)
    message("UNIX")
    include_directories(thirdparty/linux/glfw/include)
    link_directories(thirdparty/linux/glfw/lib)
elseif (WIN32)
    message("WIN32")
    include_directories(thirdparty/windows/glfw/include)
    link_directories(thirdparty/windows/glfw/lib-vc2019)
else()
    message(FATAL_ERROR "Unrecognized platform")
endif()


# GLAD
include_directories(thirdparty/glad/include)
set(SOURCE_FILES thirdparty/glad/src/glad.c)

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
message("YES")
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()


add_executable(TestOpenGL ${SOURCE_FILES} "TestOpenGL.cpp")


if (APPLE)
    target_link_libraries(TestOpenGL glfw ${COCOA_LIBRARY} ${OpenGL_LIBRARY} ${IOKit_LIBRARY} ${CoreVideo_LIBRARY})
elseif (UNIX)
    target_link_libraries(TestOpenGL glfw3 dl X11 pthread)
elseif (WIN32)
    target_link_libraries(TestOpenGL glfw3)
else()
    message(FATAL_ERROR "Unrecognized platform")
endif()
