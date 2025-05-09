#version 450 core

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform LineColorBlock {
    vec3 lineColor;
};

void main()
{
    FragColor = vec4(lineColor, 1.0);
}