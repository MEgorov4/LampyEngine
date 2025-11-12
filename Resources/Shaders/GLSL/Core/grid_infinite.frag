#version 450 core

in vec3 worldPos;
in vec3 viewDir;

layout(std140, binding = 0) uniform CameraData {
    mat4 view;
    mat4 projection;
    vec4 position;
};

out vec4 FragColor;

// Базовые настройки сетки (всё в шейдере, без GridData UBO)
const float baseStep = 1.0;
const float majorStep = 10.0;
const float fadeDistance = 200.0;
const float lineThickness = 1.5;

// Цвета
const vec3 colorMinor = vec3(0.25);
const vec3 colorMajor = vec3(0.45);
const vec3 colorAxisX = vec3(1.0, 0.2, 0.2);
const vec3 colorAxisZ = vec3(0.2, 0.4, 1.0);

// Функция для вычисления антиалиасинга линий сетки
float gridFactor(vec2 coord, float step, float thickness)
{
    vec2 grid = abs(fract(coord / step - 0.5) - 0.5) / fwidth(coord / step);
    float line = min(grid.x, grid.y);
    return 1.0 - smoothstep(0.0, thickness, line);
}

void main()
{
    // Простой тест - рисуем красный цвет чтобы увидеть, рендерится ли quad вообще
    FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Красный для максимальной видимости
    
    // Если это работает, раскомментируем код сетки ниже
    /*
    float dist = length(position.xyz - worldPos);
    
    // Упрощенный шаг для отладки - фиксированный размер
    float step = baseStep;
    float majorStepScaled = majorStep;
    
    // Anti-aliased grid lines
    float minor = gridFactor(worldPos.xz, step, lineThickness);
    float major = gridFactor(worldPos.xz, majorStepScaled, lineThickness * 1.5);
    
    vec3 color = mix(colorMinor, colorMajor, major);
    
    // Оси X и Z (более широкие для видимости)
    if (abs(worldPos.x) < step * 2.0) color = colorAxisZ;
    if (abs(worldPos.z) < step * 2.0) color = colorAxisX;
    
    // Упрощенный fade - убираем для отладки
    float fade = 1.0;
    
    // Яркие цвета для отладки
    color *= 3.0;
    
    // Полная непрозрачность
    FragColor = vec4(color, 1.0);
    */
}

