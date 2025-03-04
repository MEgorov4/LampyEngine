#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 2) in vec3 inNormal;

layout(std140, binding = 0) uniform CameraData {
    mat4 view;
    mat4 projection;
    vec4 position;
};

layout(std140, binding = 1) uniform ModelMatrices {
    mat4 model;
};

layout (location = 0) out vec3 fragWorldPos;
layout (location = 1) out vec3 fragNormal;

void main() {
    mat4 worldMatrix = model;
    vec4 worldPos = worldMatrix * vec4(inPosition, 1.0);
    fragWorldPos = worldPos.xyz;
    fragNormal = normalize(mat3(transpose(inverse(worldMatrix))) * inNormal);
    
    gl_Position = projection * view * worldPos;
}
