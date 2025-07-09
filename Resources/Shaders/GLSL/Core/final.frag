#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragUV;

layout(binding = 0) uniform sampler2D shadow_pass_depth;
layout(binding = 1) uniform sampler2D reflection_pass_depth;
layout(binding = 2) uniform sampler2D light_pass_color;
layout(binding = 3) uniform sampler2D texture_pass_color;

void main() {
    vec3 albedo = texture(texture_pass_color, fragUV).rgb;

    float shadow = texture(shadow_pass_depth, fragUV).r;
    vec3 reflection = texture(reflection_pass_depth, fragUV).rgb;
    vec3 light = texture(light_pass_color, fragUV).rgb;

    outColor = vec4(albedo, 1.0);
}
