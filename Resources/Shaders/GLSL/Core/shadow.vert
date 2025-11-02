#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(std140, binding = 0) uniform CameraData {
    mat4 view;        // lightViewMatrix
    mat4 projection;  // lightProjectionMatrix
    vec4 position;    // unused for shadows
};

// Per-object uniform (не UBO, так как меняется для каждого объекта)
uniform mat4 model;

void main() {
    mat4 worldMatrix = model;
    gl_Position = projection * view * worldMatrix * vec4(inPosition, 1.0);
}
