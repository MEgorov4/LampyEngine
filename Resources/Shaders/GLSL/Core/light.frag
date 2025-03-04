#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNormal;

layout(std140, binding = 2) uniform DirectionalLightData {
    vec3 lightDir;
    vec3 lightColor;
};

layout(std140, binding = 3) uniform PointLights {
    int lightCount;
    vec3 pointPositions[100];
    vec3 pointColors[100];
};

void main() {
    vec3 norm = normalize(fragNormal);
    vec3 light = vec3(0.0);

    // Направленный свет
    float diff = max(dot(norm, -lightDir), 0.0);
    light += diff * lightColor;

    // Точечные источники
    for (int i = 0; i < lightCount; i++) {
        vec3 lightDir = normalize(pointPositions[i] - fragWorldPos);
        float diff = max(dot(norm, lightDir), 0.0);
        light += diff * pointColors[i];
    }

    outColor = vec4(light, 1.0);
}
