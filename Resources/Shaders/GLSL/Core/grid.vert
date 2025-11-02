#version 450 core

layout(location = 0) in vec3 aPos;

layout(std140, binding = 0) uniform CameraData {
    mat4 view;
    mat4 projection;
    vec4 position;
};

layout(std140, binding = 1) uniform GridData {
    float gridSize;
    float gridStep;
    float majorStep;
    vec3 majorLineColor;
    vec3 minorLineColor;
    vec3 axisColorX;
    vec3 axisColorZ;
};

out vec3 fragPos;

void main()
{
    fragPos = aPos;
    gl_Position = projection * view * vec4(aPos, 1.0);
}

