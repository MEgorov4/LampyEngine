#version 450 core

in vec3 worldPos;
in vec3 viewDir;

layout(std140, binding = 0) uniform CameraData {
    mat4 view;
    mat4 projection;
    vec4 position;
};

out vec4 FragColor;

// базовые настройки
const float baseStep = 1.0;
const float majorStep = 10.0;
const float fadeDistance = 200.0;
const float lineThickness = 1.5;

// цвета
const vec3 colorMinor = vec3(0.25);
const vec3 colorMajor = vec3(0.45);
const vec3 colorAxisX = vec3(1.0, 0.2, 0.2);
const vec3 colorAxisZ = vec3(0.2, 0.4, 1.0);

float gridFactor(vec2 coord, float step, float thickness)
{
    vec2 grid = abs(fract(coord / step - 0.5) - 0.5) / fwidth(coord / step);
    float line = min(grid.x, grid.y);
    return 1.0 - smoothstep(0.0, thickness, line);
}

void main()
{
    float dist = length(position.xyz - worldPos);

    // динамический шаг
    float step = baseStep * pow(2.0, floor(log2(dist * 0.1 + 1.0)));
    float majorStepScaled = majorStep * step;

    // anti-aliased grid lines
    float minor = gridFactor(worldPos.xz, step, lineThickness);
    float major = gridFactor(worldPos.xz, majorStepScaled, lineThickness * 1.5);

    vec3 color = mix(colorMinor, colorMajor, major);

    // оси
    if (abs(worldPos.x) < step * 0.5) color = colorAxisZ;
    if (abs(worldPos.z) < step * 0.5) color = colorAxisX;

    // fade на дистанции
    float fade = clamp(1.0 - dist / fadeDistance, 0.0, 1.0);
    fade = smoothstep(0.0, 1.0, fade);

    // fade по углу — чем ближе к горизонту, тем сильнее исчезает
    float horizonFade = pow(1.0 - abs(viewDir.y), 4.0);
    fade *= 1.0 - horizonFade;

    FragColor = vec4(color * fade, fade);
}
