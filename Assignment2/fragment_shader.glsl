#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 objectColor;
uniform sampler2D diffuseMap;
uniform int uTransparent; // 0 = Phong, 1 = glass-like transparent

// Compute scalar Phong terms (ambient, diffuse, specular)
void computePhongScalar(
    vec3 N,
    float ambientStrength,
    float diffuseStrength,
    float specularStrength,
    float shininess,
    float lightStrength,
    float ambientLightStrength,
    out float ambient,
    out float diffuse,
    out float specular
) {
    // TODO 2: Implement Blinn-Phong/Phong lighting model
    // Some useful functions:
    // - normalize(vec3 v): returns a normalized vector
    // - dot(vec3 a, vec3 b): returns the dot product of two vectors
    // - pow(float x, float y): returns x raised to the power of y
    // - max(float x, float y): returns the maximum value between x and y
    // - reflect(vec3 v, vec3 n): returns the reflection direction of the incident vector v and the normal n

    // ambient =
    // diffuse =
    // specular =
}

void main() {

    // Common values
    vec3 Nw = normalize(Normal);
    vec3 N  = gl_FrontFacing ? Nw : -Nw;
    vec3 albedo = texture(diffuseMap, TexCoords).rgb;


    // Shared Phong color
    const float ambientStrength = 0.1;
    const float specularStrength = 0.5;
    const float diffuseStrength = 1.0;
    const float shininess = 32.0;
    const float lightStrength = 1.0;
    const float ambientLightStrength = 1.0;
    
    const float F0 = 0.02; 
    const float alphaBase = 0.25;

    float ambient, diffuse, specular;
    computePhongScalar(N,
        ambientStrength, diffuseStrength, specularStrength,
        shininess, lightStrength, ambientLightStrength,
        ambient, diffuse, specular);

    vec3 color = (ambient + diffuse + specular) * albedo;

    // Alpha: opaque=1.0; transparent uses Schlick Fresnel only for alpha
    float alpha = 1.0;
    if (uTransparent != 0) {
        float F = 1;
        // TODO 3: Compute Schlick Fresnel F for alphas

        alpha = clamp(mix(alphaBase, 1.0, F), 0.0, 1.0);
    }

    FragColor = vec4(color, alpha);
}