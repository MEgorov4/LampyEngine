#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(std140, binding = 0) uniform CameraData {
    mat4 lightView;
    mat4 lightProjection;
    vec4 position;
};

layout(std140, binding = 1) uniform ModelMatrices {
    mat4 model;
};

void main() {
    mat4 worldMatrix = model;
    gl_Position = lightProjection * lightView * worldMatrix * vec4(inPosition, 1.0);
}
