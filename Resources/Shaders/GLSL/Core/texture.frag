#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec3 fragLocalPos;

layout(binding = 0) uniform sampler2D albedoTexture;
layout(std140, binding = 1) uniform ModelMatrices {
    mat4 model;
};

const float tiling = 1.0; 

void main() {
    vec3 scale;
    scale.x = length(model[0].xyz);
    scale.y = length(model[1].xyz);
    scale.z = length(model[2].xyz);

    vec2 adjustedUV = fragLocalPos.xz * scale.xz * tiling;
    vec2 tiledUV = fract(adjustedUV);

    vec4 tex = texture(albedoTexture, fragUV);

    if (tex.rgb == vec3(0.0)) {
        vec2 g = abs(fract(adjustedUV) - 0.5);
        float line = step(0.48, max(g.x, g.y));
        outColor = vec4(mix(vec3(0.9), vec3(0.3), line), 1.0);
    } else {
        outColor = tex;
    }
}
