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

// Цвета (увеличиваем яркость для видимости)
const vec3 colorMinor = vec3(0.8); // Очень яркие минорные линии
const vec3 colorMajor = vec3(1.0); // Белые мажорные линии
const vec3 colorAxisX = vec3(1.0, 0.5, 0.5); // Ярко-красная ось X
const vec3 colorAxisZ = vec3(0.5, 0.5, 1.0); // Ярко-синяя ось Z

// Функция для вычисления антиалиасинга линий сетки
float gridFactor(vec2 coord, float step, float thickness)
{
    vec2 grid = abs(fract(coord / step - 0.5) - 0.5) / fwidth(coord / step);
    float line = min(grid.x, grid.y);
    return 1.0 - smoothstep(0.0, thickness, line);
}

void main()
{
    // ПРОСТОЙ ТЕСТ: рисуем яркий зеленый цвет для проверки видимости quad
    // Если этот цвет виден, значит quad рендерится и проблема в вычислениях сетки
    FragColor = vec4(0.0, 1.0, 0.0, 1.0); // Ярко-зеленый
    
    // Если зеленый виден, раскомментируйте код ниже для сетки
    /*
    float dist = length(position.xyz - worldPos);
    
    // Динамический шаг сетки в зависимости от расстояния
    float step = baseStep * pow(2.0, floor(log2(max(dist * 0.1, 1.0))));
    float majorStepScaled = majorStep * step;
    
    // Anti-aliased grid lines
    float minor = gridFactor(worldPos.xz, step, lineThickness);
    float major = gridFactor(worldPos.xz, majorStepScaled, lineThickness * 1.5);
    
    vec3 color = mix(colorMinor, colorMajor, major);
    
    // Оси X и Z
    if (abs(worldPos.x) < step * 2.0) color = colorAxisZ;
    if (abs(worldPos.z) < step * 2.0) color = colorAxisX;
    
    // Убираем fade полностью для максимальной видимости
    float fade = 1.0;
    
    // Минимальный fade только на очень больших дистанциях
    if (dist > fadeDistance * 3.0)
    {
        fade = 0.8;
    }
    
    // Максимальная яркость
    color *= 2.0;
    
    FragColor = vec4(color * fade, 1.0);
    */
}

