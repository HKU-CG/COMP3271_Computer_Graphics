set_project("COMP3271")
add_rules("mode.release", "mode.debug")
set_languages("cxx20")

add_requires("glad 0.1.36", "glfw", "glm")
add_requires("imgui", { configs = { glfw_opengl3 = true } })
add_requires("spdlog")
add_requires("stb")
add_requires("tinyobjloader")

if is_plat("windows") then
	add_cxxflags("/utf-8")
end

if is_plat("macosx") then
	add_defines("PLATFORM_MACOSX")
end

target("engine")
set_kind("headeronly")
add_packages("glad", "glfw", "glm", { public = true })
add_packages("spdlog", { public = true })
add_packages("imgui", { public = true })
add_packages("stb", { public = true })
add_packages("tinyobjloader", { public = true })
add_headerfiles("$(projectdir)/engine/(**.hpp)")
add_includedirs("$(projectdir)/engine/", { public = true })
target_end()

target("test")
set_kind("binary")
add_deps("engine")
add_files("$(projectdir)/test/opengl_demo.cpp")
target_end()

includes("Assignment1/xmake.lua")
includes("Assignment2/xmake.lua")
