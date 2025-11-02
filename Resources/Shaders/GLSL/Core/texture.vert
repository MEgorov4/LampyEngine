#version 450 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec3 fragWorldPos;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragTangent;
layout(location = 4) out vec3 fragBitangent;
layout(location = 5) out vec3 fragLocalPos;

layout(std140, binding = 0) uniform CameraData {
    mat4 view;
    mat4 projection;
    vec4 position;
};

// Per-object uniforms (не UBO, так как меняются для каждого объекта)
uniform mat4 model;
uniform mat3 normalMatrix; // Precomputed normal matrix (transpose(inverse(model)))

void main() 
{
    fragUV = inUV;
    fragLocalPos = inPosition;
    
    vec4 worldPos = model * vec4(inPosition, 1.0);
    fragWorldPos = worldPos.xyz;
    
    // Transform normals to world space
    // normalMatrix передаётся из CPU (предвычислен), что экономит ALU
    // Трансформируем нормаль - всегда нормализуем для единичного вектора
    fragNormal = normalize(normalMatrix * inNormal);
    
    // Transform tangent to world space
    // Используем проверку длины через dot для оптимизации (меньше ALU чем length)
    float tangentLenSq = dot(inTangent, inTangent);
    if (tangentLenSq > 0.000001) { // 0.001^2 вместо length(inTangent) > 0.001
        // Трансформируем тангент в мировое пространство
        fragTangent = normalize(normalMatrix * inTangent);
        
        // Пересчитываем bitangent чтобы он был ортогонален к normal и tangent
        // Это важно для правильной работы нормальных карт при поворотах
        fragBitangent = normalize(cross(fragNormal, fragTangent));
        
        // Ортогонализация TBN матрицы методом Грама-Шмидта
        // Убираем компоненту тангента, параллельную нормали
        fragTangent = normalize(fragTangent - dot(fragTangent, fragNormal) * fragNormal);
        
        // Пересчитываем bitangent чтобы гарантировать ортогональность
        fragBitangent = normalize(cross(fragNormal, fragTangent));
    } else {
        // Если тангент отсутствует, создаем ортогональную базу
        fragTangent = normalize(vec3(1.0, 0.0, 0.0));
        // Если тангент параллелен нормали, выбираем другой вектор
        if (abs(dot(fragNormal, fragTangent)) > 0.9) {
            fragTangent = normalize(vec3(0.0, 1.0, 0.0));
        }
        fragBitangent = normalize(cross(fragNormal, fragTangent));
        fragTangent = normalize(cross(fragBitangent, fragNormal));
    }
    
    gl_Position = projection * view * worldPos;
}
