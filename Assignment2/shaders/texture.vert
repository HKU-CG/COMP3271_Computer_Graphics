#version 330 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform float u_uv_scale;

out vec2 v_uv;
out vec3 v_world_normal;
out vec3 v_world_position;

void main() {
    // ===== STUDENT_TASK_BEGIN: assignment2_texture_vertex_shader =====
    gl_Position = vec4(0.0);
    v_uv = vec2(0.0);
    v_world_position = vec3(0.0);
    v_world_normal = vec3(0.0);
    // ===== STUDENT_TASK_END: assignment2_texture_vertex_shader =====
}
