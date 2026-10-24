#include "assignment2_app.hpp"

#include "gl_base.hpp"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <iostream>

int main() {
    try {
        GLFWContext glfw_context;
        GLWindow window(1440, 900, "Assignment 2 - Shader Lab");
        glfwSwapInterval(1);
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().IniFilename = nullptr;
        ImFontConfig font_config;
        font_config.SizePixels = 21.0F;
        ImGui::GetIO().Fonts->AddFontDefault(&font_config);
        ImGui::StyleColorsDark();
        ImGui::GetStyle().ScaleAllSizes(1.15F);
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330 core");
        {
            Assignment2App app;
            while (!window.should_close()) {
                window.poll_events();
                if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                }
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();
                app.draw();
                ImGui::Render();
                int width = 0, height = 0;
                window.get_framebuffer_size(width, height);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glViewport(0, 0, width, height);
                glClearColor(0.07F, 0.08F, 0.10F, 1.0F);
                glClear(GL_COLOR_BUFFER_BIT);
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                window.swap_buffers();
            }
        }
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    } catch (const std::exception &error) {
        std::cerr << "Assignment 2 failed: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
