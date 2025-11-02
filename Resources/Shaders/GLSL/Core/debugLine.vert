#version 450 core

layout(location = 0) in vec3 aPos;

layout(std140, binding = 0) uniform CameraData {
    mat4 view;
    mat4 projection;
    vec4 position;
};

// Per-object uniform (не UBO, так как меняется для каждого объекта)
uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}