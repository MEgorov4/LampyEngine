#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 viewDir;

layout(std140, binding = 2) uniform LightData {
    vec3 lightDir;
    vec3 lightColor;
};

void main() {
    vec3 reflectedDir = reflect(-viewDir, fragNormal);
    float spec = pow(max(dot(reflectedDir, lightDir), 0.0), 32.0);

    vec3 reflectionColor = lightColor * spec;
    outColor = vec4(reflectionColor, 1.0);
}
