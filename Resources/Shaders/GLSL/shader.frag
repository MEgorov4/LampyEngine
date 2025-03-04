#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragWorldPos;

layout(location = 0) out vec4 FragColor;

void main() {
    vec3 normalColor = fragNormal * 0.5 + 0.5;
    FragColor = vec4(fragUV.x, fragUV.y, 0, 1.0);
}
