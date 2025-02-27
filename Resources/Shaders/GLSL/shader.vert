#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(std140, binding = 0) uniform ShaderData {
    mat4 model;
    mat4 view;
    mat4 projection;
};

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragWorldPos;

void main() {
    vec4 worldPosition = model * vec4(inPosition, 1.0);
    gl_Position = projection * view * worldPosition;

    fragUV = inUV;
    fragNormal = normalize(mat3(transpose(inverse(model))) * inNormal); 
    fragWorldPos = worldPosition.xyz;
}
