#version 450 core

in vec3 fragPos;

layout(std140, binding = 1) uniform GridData {
    float gridSize;
    float gridStep;
    float majorStep;
    vec3 majorLineColor;
    vec3 minorLineColor;
    vec3 axisColorX;
    vec3 axisColorZ;
};

out vec4 FragColor;

void main()
{
    vec3 pos = fragPos;
    
    // Определяем, на какой линии мы находимся
    float x = pos.x;
    float z = pos.z;
    
    // Определяем цвет в зависимости от того, на какой линии мы находимся
    vec3 color = minorLineColor;
    
    // Проверяем, находится ли точка на оси или близко к ней
    if (abs(x) < 0.01)
    {
        color = axisColorX;
    }
    else if (abs(z) < 0.01)
    {
        color = axisColorZ;
    }
    else
    {
        // Проверяем, является ли линия основной (кратна majorStep)
        // Упрощённое вычисление с mod вместо floor (меньше ALU)
        const float epsilon = 0.02;
        float xMod = mod(abs(x), majorStep);
        float zMod = mod(abs(z), majorStep);
        
        bool isMajorX = (xMod < epsilon || xMod > (majorStep - epsilon));
        bool isMajorZ = (zMod < epsilon || zMod > (majorStep - epsilon));
        
        if (isMajorX || isMajorZ)
        {
            color = majorLineColor;
        }
    }
    
    FragColor = vec4(color, 1.0);
}

