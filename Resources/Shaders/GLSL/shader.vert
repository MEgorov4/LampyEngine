#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(location = 3) uniform mat4 model;
layout(location = 4) uniform mat4 viewProjection;
layout(location = 5) uniform mat3 normalMatrix;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec3 fragNormal;

void main()
{
    gl_Position = viewProjection * (model * vec4(inPosition, 1.0));
    
    fragUV = inUV;
    
    fragNormal = normalize(normalMatrix * inNormal);
}
