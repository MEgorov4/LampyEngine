#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragUV;

layout(binding = 0) uniform sampler2D shadow_pass_depth;
layout(binding = 1) uniform sampler2D optionalReflection; // reflection_pass_depth (опционально, не используется)
layout(binding = 2) uniform sampler2D texture_pass_color; // Основной PBR результат

void main() {
    // texture_pass_color уже содержит полный PBR результат с тенями и освещением
    vec3 finalColor = texture(texture_pass_color, fragUV).rgb;
    
    // Можно добавить отражения если есть reflection pass
    // vec3 reflection = texture(reflection_pass_depth, fragUV).rgb;
    // finalColor += reflection;
    
    // Tone mapping уже применен в texture_pass, но можно добавить дополнительную обработку
    // Gamma correction также уже применен
    
    outColor = vec4(finalColor, 1.0);
}
