#version 450 core

layout(location = 0) in vec3 inPos;

layout(std140, binding = 0) uniform CameraData {
    mat4 view;
    mat4 projection;
    vec4 position; // world-space camera position
};

out vec3 worldPos;
out vec3 viewDir;

void main()
{
    // Позиция камеры в мировом пространстве
    vec3 camPos = position.xyz;

    // Используем большой quad (-1..1) в clip space, чтобы сделать "бесконечную" плоскость
    vec2 uv = inPos.xz * 2.0 - 1.0; // ожидается quad с координатами (0..1)
    vec4 clip = vec4(uv, 0.0, 1.0);

    // Обратная проекция — чтобы получить world-pos прямо на "земле" (y=0)
    mat4 invProj = inverse(projection);
    mat4 invView = inverse(view);

    vec4 worldRay = invView * invProj * clip;
    worldRay /= worldRay.w;
    
    // Пересечение луча камеры с плоскостью Y=0
    float t = -camPos.y / normalize(worldRay.xyz - camPos).y;
    worldPos = camPos + normalize(worldRay.xyz - camPos) * t;

    viewDir = normalize(camPos - worldPos);
    gl_Position = vec4(uv, 0.0, 1.0); // всегда в пределах фрейма
}
