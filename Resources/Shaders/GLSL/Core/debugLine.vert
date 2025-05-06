#version 450 core

layout(location = 0) in vec3 aPos;

layout(binding = 0) uniform Matrices {
    mat4 projection;
    mat4 view;
    mat4 model;
};

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}