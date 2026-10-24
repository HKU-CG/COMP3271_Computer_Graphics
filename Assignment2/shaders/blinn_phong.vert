#version 330 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 v_world_position;
out vec3 v_world_normal;

void main() {
    // ===== STUDENT_TASK_BEGIN: assignment2_blinn_phong_vertex_shader =====
    gl_Position = vec4(0.0);
    v_world_position = vec3(0.0);
    v_world_normal = vec3(0.0);
    // ===== STUDENT_TASK_END: assignment2_blinn_phong_vertex_shader =====
}
