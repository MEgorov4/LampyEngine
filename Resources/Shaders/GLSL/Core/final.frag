#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragUV;

layout(binding = 0) uniform sampler2D shadowMap;
layout(binding = 1) uniform sampler2D reflectionMap;
layout(binding = 2) uniform sampler2D lightMap;
layout(binding = 3) uniform sampler2D customMap;
layout(binding = 4) uniform sampler2D objectAlbedo;
layout(binding = 5) uniform sampler2D objectNormal;
layout(binding = 6) uniform sampler2D objectSpecular;
layout(binding = 7) uniform sampler2D objectRoughness;
layout(binding = 8) uniform sampler2D objectEmission;

void main() {
    vec3 albedo = texture(objectAlbedo, fragUV).rgb;
    vec3 normal = texture(objectNormal, fragUV).rgb * 2.0 - 1.0;
    vec3 specular = texture(objectSpecular, fragUV).rgb;
    float roughness = texture(objectRoughness, fragUV).r;
    vec3 emission = texture(objectEmission, fragUV).rgb;

    float shadow = texture(shadowMap, fragUV).r;
    vec3 reflection = texture(reflectionMap, fragUV).rgb;
    vec3 light = texture(lightMap, fragUV).rgb;
    vec3 custom = texture(customMap, fragUV).rgb;

    outColor = vec4((light * shadow + reflection + emission) * albedo * custom, 1.0);
}
