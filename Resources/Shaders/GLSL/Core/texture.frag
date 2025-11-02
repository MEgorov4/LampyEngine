#version 450 core

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec3 fragWorldPos;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragTangent;
layout(location = 4) in vec3 fragBitangent;
layout(location = 5) in vec3 fragLocalPos;

layout(binding = 0) uniform sampler2D albedoTexture;
layout(binding = 1) uniform sampler2D normalTexture;
layout(binding = 2) uniform sampler2D roughnessMetallicTexture;
layout(binding = 3) uniform sampler2D shadowMap;

layout(std140, binding = 0) uniform CameraData {
    mat4 view;
    mat4 projection;
    vec4 position;
};

// ModelMatrices не используется в fragment shader

layout(std140, binding = 2) uniform DirectionalLightData {
    vec4 lightDirection;
    vec4 lightColor;
    float lightIntensity;
    float padding[3];
};

layout(std140, binding = 3) uniform MaterialData {
    vec4 albedoColor;
    float roughness;
    float metallic;
    float normalStrength;
    float padding2;
};

layout(std140, binding = 4) uniform LightSpaceMatrix {
    mat4 lightSpaceMatrix;
};

// Point lights через обычные uniform массивы (не UBO)
uniform int lightCount;
uniform vec4 pointPositions[100];
uniform vec4 pointColors[100];
uniform float pointIntensities[100];
uniform float pointInnerRadii[100]; // Внутренние радиусы (полная интенсивность)
uniform float pointOuterRadii[100]; // Внешние радиусы (затухание до 0)

const float PI = 3.14159265359;
const float tiling = 1.0;

// Shadow calculation
float ShadowCalculation(vec4 fragPosLightSpace, vec3 N, vec3 lightDir)
{
    // Perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Get depth of current fragment from light's perspective
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    
    // Shadow bias варьируется по углу (меньше bias для поверхностей, перпендикулярных свету)
    // Это предотвращает shadow acne без создания peter panning
    float NdotL = max(dot(N, lightDir), 0.0);
    float bias = max(0.005 * (1.0 - NdotL), 0.001);
    
    // PCF (Percentage Closer Filtering) for softer shadows
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    // Keep the shadow at 0.0 when outside the far_plane region of the light's frustum
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

// Trowbridge-Reitz GGX normal distribution
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return num / max(denom, 0.0000001);
}

// Schlick-GGX geometry function
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return num / max(denom, 0.0000001);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 calculateNormal()
{
    vec3 tangentNormal = texture(normalTexture, fragUV).xyz * 2.0 - 1.0;
    tangentNormal.xy *= normalStrength;
    
    // Flip Y для совместимости OpenGL vs DirectX normal maps
    #ifdef FLIP_NORMAL_Y
    tangentNormal.y = -tangentNormal.y;
    #endif
    
    tangentNormal = normalize(tangentNormal);
    
    mat3 TBN = mat3(fragTangent, fragBitangent, fragNormal);
    return normalize(TBN * tangentNormal);
}

void main() 
{
    // Get material properties
    vec4 albedo = texture(albedoTexture, fragUV);
    if (albedo.rgb == vec3(0.0)) {
        vec2 adjustedUV = fragLocalPos.xz * tiling;
        vec2 g = abs(fract(adjustedUV) - 0.5);
        float line = step(0.48, max(g.x, g.y));
        albedo = vec4(mix(vec3(0.9), vec3(0.3), line), 1.0);
    }
    
    albedo.rgb *= albedoColor.rgb;
    
    vec4 roughnessMetallic = texture(roughnessMetallicTexture, fragUV);
    float rough = clamp(roughness * roughnessMetallic.g, 0.0, 1.0);
    float metal = metallic * roughnessMetallic.b;
    
    // Calculate normal
    vec3 N = calculateNormal();
    
    // View direction
    vec3 V = normalize(position.xyz - fragWorldPos);
    
    // Light direction (directional light)
    vec3 L = normalize(-lightDirection.xyz);
    
    // Shadow calculation
    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fragWorldPos, 1.0);
    float shadow = ShadowCalculation(fragPosLightSpace, N, L);
    
    // Half vector
    vec3 H = normalize(V + L);
    
    // Calculate F0 for fresnel
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo.rgb, metal);
    
    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, rough);
    float G = GeometrySmith(N, V, L, rough);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metal;
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    float NdotL = max(dot(N, L), 0.0);
    vec3 Lo = (kD * albedo.rgb / PI + specular) * lightColor.rgb * lightIntensity * NdotL * (1.0 - shadow);
    
    // Point lights contribution
    for (int i = 0; i < lightCount && i < 100; i++)
    {
        vec3 toLight = pointPositions[i].xyz - fragWorldPos;
        float distance = length(toLight);
        vec3 lightDir = normalize(toLight); // Направление от фрагмента к источнику света (для diffuse/specular нужен вектор от источника к фрагменту)
        
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
        
        // Направление света - от источника к фрагменту (обратное lightDir)
        vec3 L = -lightDir; // Направление от источника света к фрагменту
        
        // Half vector for point light
        vec3 H_point = normalize(V + L);
        
        // PBR calculation for point light
        float NDF_point = DistributionGGX(N, H_point, rough);
        float G_point = GeometrySmith(N, V, L, rough);
        vec3 F_point = fresnelSchlick(max(dot(H_point, V), 0.0), F0);
        
        vec3 kS_point = F_point;
        vec3 kD_point = vec3(1.0) - kS_point;
        kD_point *= 1.0 - metal;
        
        vec3 numerator_point = NDF_point * G_point * F_point;
        float denominator_point = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular_point = numerator_point / denominator_point;
        
        float NdotL_point = max(dot(N, L), 0.0);
        vec3 Lo_point = (kD_point * albedo.rgb / PI + specular_point) * pointColors[i].rgb * pointIntensities[i] * NdotL_point * attenuation;
        
        Lo += Lo_point;
    }
    
    // Ambient
    vec3 ambient = vec3(0.03) * albedo.rgb;
    
    vec3 color = ambient + Lo;
    
    // HDR tone mapping
    color = color / (color + vec3(1.0));
    
    // Gamma correction
    color = pow(color, vec3(1.0/2.2));
    
    outColor = vec4(color, albedo.a);
}
