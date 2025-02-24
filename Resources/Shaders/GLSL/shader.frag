#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec3 fragNormal; 

layout(location = 0) out vec4 outColor;

void main()
{
    float scale = 10.0;
    vec2 uvScaled = fragUV * scale;
    
    float checker = mod(floor(uvScaled.x) + floor(uvScaled.y), 2.0);
    
    vec3 chessColor = (checker < 1.0) ? vec3(1.0) : vec3(0.0);
    
    outColor = vec4(chessColor, 1.0);
}
