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
    // Используем их как NDC координаты для создания луча из камеры
    
    // Получаем NDC координаты из quad
    vec2 ndc = inPosition.xz;
    
    // Создаем луч в clip space
    // Используем координаты quad как экранные координаты
    vec4 rayClip = vec4(ndc.x, ndc.y, 1.0, 1.0);
    
    // Обратная проекция для получения точки в view space
    vec4 rayEye = inverse(projection) * rayClip;
    rayEye.xyz /= rayEye.w;
    
    // Направление луча в view space (нормализованное)
    vec3 rayDirView = normalize(rayEye.xyz);
    
    // Преобразуем направление в world space
    mat4 invView = inverse(view);
    vec3 rayDir = normalize((invView * vec4(rayDirView, 0.0)).xyz);
    
    // Пересекаем луч с плоскостью Y=0
    // Уравнение плоскости: y = 0
    // Уравнение луча: P = camPos + t * rayDir
    // Для пересечения: camPos.y + t * rayDir.y = 0
    // t = -camPos.y / rayDir.y
    
    float t = 0.0;
    if (abs(rayDir.y) > 0.0001)
    {
        t = -camPos.y / rayDir.y;
        
        // Если t отрицательное, камера под плоскостью - используем большую дистанцию
        if (t < 0.0)
        {
            t = 100000.0;
        }
    }
    else
    {
        // Камера смотрит горизонтально (rayDir.y близко к 0)
        t = 100000.0;
    }
    
    // Вычисляем world position на плоскости Y=0
    worldPos = camPos + rayDir * t;
    
    // Направление взгляда (от точки на плоскости к камере)
    viewDir = normalize(camPos - worldPos);
    
    // Трансформируем в clip space для правильного рендеринга
    vec4 clipPos = projection * view * vec4(worldPos, 1.0);
    gl_Position = clipPos;
}

