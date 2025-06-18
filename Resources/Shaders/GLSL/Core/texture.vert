#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec3 fragLocalPos;

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
    fragUV = inUV;
    fragLocalPos = inPosition;
    gl_Position = projection * view * (model * vec4(inPosition, 1.0));
}
