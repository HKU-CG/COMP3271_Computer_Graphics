#version 330 core

in vec2 v_uv;
in vec3 v_world_normal;
in vec3 v_world_position;

uniform sampler2D u_texture;
uniform vec3 u_base_color;
uniform vec3 u_light_pos;
uniform vec3 u_camera_pos;
uniform float u_shininess;

out vec4 fragment_color;

void main() {
    // ===== STUDENT_TASK_BEGIN: assignment2_texture_fragment_shader =====
    fragment_color = vec4(0.0);
    // ===== STUDENT_TASK_END: assignment2_texture_fragment_shader =====
}
