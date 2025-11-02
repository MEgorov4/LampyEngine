#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragUV;

layout(binding = 0) uniform sampler2D sourceTexture;

void main() {
    outColor = texture(sourceTexture, fragUV);
}
