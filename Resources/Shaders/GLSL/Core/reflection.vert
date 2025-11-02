#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(std140, binding = 0) uniform CameraData {
    mat4 view;
    mat4 projection;
    vec4 cameraPos;
};

// Per-object uniform (не UBO, так как меняется для каждого объекта)
uniform mat4 model;

layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 viewDir;

void main() {
    mat4 worldMatrix = model;
    vec4 worldPos = worldMatrix * vec4(inPosition, 1.0);
    fragWorldPos = worldPos.xyz;
    fragNormal = normalize(mat3(transpose(inverse(worldMatrix))) * inNormal);
    
    viewDir = normalize(cameraPos.xyz - fragWorldPos);
    gl_Position = projection * view * worldPos;
}
