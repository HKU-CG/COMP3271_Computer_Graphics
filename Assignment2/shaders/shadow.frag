#version 330 core

in vec3 v_world_position;
in vec3 v_world_normal;
in vec4 v_light_clip;

uniform sampler2D u_shadow_map;
uniform vec3 u_light_direction;
uniform vec3 u_camera_pos;
uniform vec3 u_base_color;
uniform float u_shininess;
uniform float u_shadow_bias;
uniform bool u_apply_shadow;

out vec4 fragment_color;

float shadow_visibility(vec4 light_clip) {
    // ===== STUDENT_TASK_BEGIN: assignment2_shadow_visibility =====
    return 1.0;
    // ===== STUDENT_TASK_END: assignment2_shadow_visibility =====
}

void main() {
    vec3 normal = normalize(v_world_normal);
    vec3 light_direction = normalize(u_light_direction);
    vec3 view_direction = normalize(u_camera_pos - v_world_position);
    vec3 half_vector = normalize(light_direction + view_direction);
    float diffuse = max(dot(normal, light_direction), 0.0);
    float specular = 0.0;
    if (diffuse > 0.0) {
        specular = pow(max(dot(normal, half_vector), 0.0), u_shininess);
    }
    float visibility = u_apply_shadow ? shadow_visibility(v_light_clip) : 1.0;
    vec3 color = 0.15 * u_base_color
               + visibility * (0.75 * diffuse * u_base_color + vec3(0.55 * specular));
    fragment_color = vec4(color, 1.0);
}
