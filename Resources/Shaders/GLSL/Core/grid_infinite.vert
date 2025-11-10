#version 450 core

layout(location = 0) in vec3 inPosition;

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
    
    // Quad координаты: inPosition.xz от -1 до 1
    // Используем их как экранные координаты (NDC)
    vec2 ndc = inPosition.xz;
    
    // Создаем луч из камеры через эту точку экрана
    // Используем подход из старого grid.vert
    vec4 rayClip = vec4(ndc.x, ndc.y, 1.0, 1.0);
    
    // Обратная проекция: clip -> view space
    mat4 invProj = inverse(projection);
    mat4 invView = inverse(view);
    
    vec4 rayEye = invProj * rayClip;
    rayEye.xyz /= rayEye.w;
    
    // Преобразуем в world space
    vec4 worldRay = invView * vec4(rayEye.xyz, 0.0);
    worldRay.xyz = normalize(worldRay.xyz);
    
    // Пересекаем луч с плоскостью Y=0
    // Уравнение: camPos.y + t * worldRay.y = 0
    float t = 0.0;
    if (abs(worldRay.y) > 1e-6)
    {
        t = -camPos.y / worldRay.y;
        if (t <= 0.0)
        {
            t = 100000.0; // Камера под плоскостью
        }
    }
    else
    {
        t = 100000.0; // Горизонтальный взгляд
    }
    
    // Вычисляем world position на плоскости Y=0
    worldPos = camPos + worldRay.xyz * t;
    
    // Направление взгляда
    viewDir = normalize(camPos - worldPos);
    
    // Quad всегда покрывает весь экран
    // Используем NDC координаты напрямую
    gl_Position = vec4(ndc, 0.0, 1.0);
}

