#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 viewDir;

layout(std140, binding = 2) uniform LightData {
    vec4 lightDir;   // Выровнено по std140 (vec4 вместо vec3)
    vec4 lightColor; // Выровнено по std140 (vec4 вместо vec3)
};

void main() {
    vec3 reflectedDir = reflect(-viewDir, fragNormal);
    float spec = pow(max(dot(reflectedDir, lightDir.xyz), 0.0), 32.0);

    vec3 reflectionColor = lightColor.rgb * spec;
    outColor = vec4(reflectionColor, 1.0);
}
