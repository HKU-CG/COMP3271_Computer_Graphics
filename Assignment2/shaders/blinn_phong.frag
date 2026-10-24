#version 330 core

in vec3 v_world_position;
in vec3 v_world_normal;

uniform vec3 u_light_pos;
uniform vec3 u_camera_pos;
uniform vec3 u_base_color;
uniform float u_shininess;

out vec4 fragment_color;

void main() {
    // ===== STUDENT_TASK_BEGIN: assignment2_blinn_phong_fragment_shader =====
    fragment_color = vec4(0.0);
    // ===== STUDENT_TASK_END: assignment2_blinn_phong_fragment_shader =====
}
