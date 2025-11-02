#version 450 core

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNormal;

layout(std140, binding = 0) uniform CameraData {
    mat4 view;
    mat4 projection;
    vec4 position;
};

layout(std140, binding = 2) uniform DirectionalLightData {
    vec4 lightDirection;
    vec4 lightColor;
    float lightIntensity;
    float padding[3];
};

// Point lights через обычные uniform массивы (не UBO)
uniform int lightCount;
uniform vec4 pointPositions[100];
uniform vec4 pointColors[100];
uniform float pointIntensities[100];
uniform float pointInnerRadii[100]; // Внутренние радиусы (полная интенсивность)
uniform float pointOuterRadii[100]; // Внешние радиусы (затухание до 0)

const float PI = 3.14159265359;
const float ambientStrength = 0.1;
const float specularStrength = 0.5;
const float shininess = 32.0;

void main() 
{
    vec3 N = normalize(fragNormal);
    vec3 V = normalize(position.xyz - fragWorldPos);
    vec3 light = vec3(0.0);

    // Направленный свет (улучшенная модель)
    vec3 L = normalize(-lightDirection.xyz);
    float NdotL = max(dot(N, L), 0.0);
    
    // Diffuse
    vec3 diffuse = lightColor.rgb * lightIntensity * NdotL;
    
    // Specular (Blinn-Phong)
    vec3 H = normalize(V + L);
    float NdotH = max(dot(N, H), 0.0);
    vec3 specular = lightColor.rgb * lightIntensity * pow(NdotH, shininess) * specularStrength;
    
    light += diffuse + specular;

    // Точечные источники (с затуханием)
    for (int i = 0; i < lightCount && i < 100; i++) 
    {
        vec3 toLight = pointPositions[i].xyz - fragWorldPos;
        float distance = length(toLight);
        vec3 lightDir = normalize(toLight); // Направление от фрагмента к источнику
        vec3 L = -lightDir; // Направление от источника к фрагменту (для освещения)
        
        // Улучшенное затухание с внутренним и внешним радиусом
        float innerRadius = pointInnerRadii[i];
        float outerRadius = pointOuterRadii[i];
        float attenuation = 1.0;
        
        if (distance < innerRadius) {
            // Полная интенсивность внутри внутреннего радиуса
            attenuation = 1.0;
        } else if (distance < outerRadius) {
            // Плавное затухание между внутренним и внешним радиусом
            float t = (distance - innerRadius) / max(outerRadius - innerRadius, 0.001);
            attenuation = 1.0 - smoothstep(0.0, 1.0, t);
        } else {
            // Нет света за пределами внешнего радиуса
            attenuation = 0.0;
        }
        
        // Diffuse
        float diff = max(dot(N, L), 0.0);
        vec3 diffusePoint = pointColors[i].rgb * pointIntensities[i] * diff * attenuation;
        
        // Specular
        vec3 H_point = normalize(V + L);
        float spec = pow(max(dot(N, H_point), 0.0), shininess);
        vec3 specularPoint = pointColors[i].rgb * pointIntensities[i] * spec * specularStrength * attenuation;
        
        light += diffusePoint + specularPoint;
    }
    
    // Ambient
    vec3 ambient = vec3(ambientStrength) * lightColor.rgb;
    
    vec3 color = ambient + light;
    
    // Tone mapping
    color = color / (color + vec3(1.0));
    
    outColor = vec4(color, 1.0);
}
