#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 fragUV;

layout(std140, binding = 0) uniform CameraData {
    mat4 view;
    mat4 projection;
    vec4 position;
};

layout(std140, binding = 1) uniform ModelMatrices {
    mat4 model;
};

void main() 
{
    vec4 worldPos = model * vec4(inPosition, 1.0);

    fragUV = inUV;
    gl_Position = projection * view * worldPos;
}