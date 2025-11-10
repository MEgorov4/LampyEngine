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
    vec4 position; // World-space camera position
};

// ModelMatrices не используется в fragment shader

layout(std140, binding = 2) uniform DirectionalLightData {
    vec4 lightDirection; // World-space light direction (normalized, direction from light source)
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

// Light-space matrix for shadow mapping (separate from world-space lighting)
// Used only for shadow calculation, not for BRDF lighting
layout(std140, binding = 4) uniform LightSpaceMatrix {
    mat4 lightSpaceMatrix; // Transforms world-space positions to light-space for shadow mapping
};

// Point lights через обычные uniform массивы (не UBO)
uniform int lightCount;
uniform vec4 pointPositions[100]; // World-space point light positions
uniform vec4 pointColors[100];
uniform float pointIntensities[100];
uniform float pointInnerRadii[100]; // Внутренние радиусы (полная интенсивность)
uniform float pointOuterRadii[100]; // Внешние радиусы (затухание до 0)

// Debug mode: 0 = normal rendering, 1-9 = debug visualization modes
uniform int debugMode;

const float PI = 3.14159265359;
const float tiling = 1.0;

// Shadow calculation
// Note: This function uses light-space coordinates (separate from world-space lighting)
// fragPosLightSpace is in light-space, but N and lightDir are in world-space for bias calculation
float ShadowCalculation(vec4 fragPosLightSpace, vec3 N, vec3 lightDir)
{
    // Perspective divide (light-space to NDC)
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range (light-space NDC to texture coordinates)
    projCoords = projCoords * 0.5 + 0.5;
    
    // Get depth of current fragment from light's perspective
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    
    // Shadow bias варьируется по углу (меньше bias для поверхностей, перпендикулярных свету)
    // Это предотвращает shadow acne без создания peter panning
    // N and lightDir are in world-space (same space as lighting calculations)
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

// Returns world-space normal
// All inputs (fragTangent, fragBitangent, fragNormal) are in world-space from vertex shader
// If normal map is not provided, normalStrength will be 0 and this function returns geometric normal
vec3 calculateNormal()
{
    // If normal map is disabled/absent (normalStrength <= 0.001), use geometric normal
    // This is the fast path and avoids TBN calculations and texture sampling
    // CPU sets normalStrength = 0 when normal texture is not bound
    if (normalStrength <= 0.001)
        return normalize(fragNormal);                   // Geometric world-space normal (no normal map)

    // Normal map is enabled - sample and transform to world-space
    // This code path is only reached if normalStrength > 0.001 AND texture is bound
    vec3 tangentNormal = texture(normalTexture, fragUV).xyz * 2.0 - 1.0; // Sample tangent-space normal
    tangentNormal.xy *= normalStrength;                // Scale XY by strength
#ifdef FLIP_NORMAL_Y
    tangentNormal.y = -tangentNormal.y;                // Optional Y-flip for DirectX-style maps
#endif
    tangentNormal = normalize(tangentNormal);          // Normalize sampled normal

    // Build TBN basis matrix from world-space vectors
    // TBN transforms from tangent-space to world-space
    // Note: In GLSL, mat3(vec3, vec3, vec3) creates a matrix with vectors as COLUMNS
    // So mat3(T, B, N) means: column0=T, column1=B, column2=N
    // This is correct for transforming tangent-space normal to world-space: worldNormal = TBN * tangentNormal
    mat3 TBN = mat3(fragTangent, fragBitangent, fragNormal); // TBN basis in world-space
    return normalize(TBN * tangentNormal);             // Transform to world-space and normalize
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
    
    // All lighting calculations are performed in world-space
    // Calculate normal in world-space
    vec3 N = calculateNormal(); // World-space normal
    
    // View direction in world-space: from fragment to camera
    vec3 V = normalize(position.xyz - fragWorldPos); // V in world-space
    
    // Light direction in world-space (directional light)
    vec3 L = normalize(-lightDirection.xyz); // L in world-space: direction from light source to fragment
    
    // Shadow calculation (separate from lighting - uses light-space)
    // Convert world-space fragment position to light-space for shadow mapping
    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fragWorldPos, 1.0);
    float shadow = ShadowCalculation(fragPosLightSpace, N, L);
    
    // Half vector in world-space (for BRDF calculations)
    vec3 H = normalize(V + L); // H in world-space: halfway between V and L
    
    // Calculate F0 for fresnel
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo.rgb, metal);
    
    // Cook-Torrance BRDF (all vectors in world-space: N, V, L, H)
    // All dot products (N·L, N·V, H·V) are correct because all vectors are in the same space
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
    
    // Point lights contribution (all calculations in world-space)
    for (int i = 0; i < lightCount && i < 100; i++)
    {
        vec3 lightPos = pointPositions[i].xyz;
        float dist = length(lightPos - fragWorldPos);
        
        // Light direction: from fragment to light source (consistent with directional light)
        // This matches the standard PBR convention where L points from fragment to light
        vec3 L = normalize(lightPos - fragWorldPos);
        
        // Physically correct inverse-square falloff with smooth outer fade
        float attenuation = 1.0 / (dist * dist);
        // Smooth fade: full intensity at innerRadius, fades to 0 at outerRadius
        // smoothstep(outer, inner, dist) returns 1.0 when dist <= inner, 0.0 when dist >= outer
        attenuation *= smoothstep(pointOuterRadii[i], pointInnerRadii[i], dist);
        
        // Calculate NdotL: standard PBR convention (L points from fragment to light)
        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0) continue; // Early exit: no contribution from back-facing surfaces
        
        // PBR lighting calculations (all vectors in world-space: N, V, L, H)
        // Standard PBR: H is halfway between V (to camera) and L (to light)
        vec3 H = normalize(V + L);
        float NDF = DistributionGGX(N, H, rough);
        float G = GeometrySmith(N, V, L, rough);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
        vec3 kS = F;
        vec3 kD = (1.0 - kS) * (1.0 - metal);
        vec3 numerator = NDF * G * F;
        float NdotV = max(dot(N, V), 0.0);
        float denom = 4.0 * NdotV * NdotL + 0.001;
        vec3 specular = numerator / denom;
        
        vec3 Lo_point = (kD * albedo.rgb / PI + specular)
            * pointColors[i].rgb * pointIntensities[i] * NdotL * attenuation;
        
        Lo += Lo_point;
    }
    
    // Ambient
    vec3 ambient = vec3(0.03) * albedo.rgb;
    
    vec3 color = ambient + Lo;
    
    // ============================================================
    // DEBUG MODES: Visualize different aspects of the rendering pipeline
    // Set debugMode uniform to switch between modes:
    // 0 = normal rendering
    // 1 = visualize normals (fragNormal - geometric normal only)
    // 2 = visualize normal length (check unit length)
    // 3 = visualize tangents
    // 4 = visualize bitangents
    // 5 = visualize TBN orthogonality
    // 6 = visualize NdotL for point light (uses geometric normal)
    // 7 = visualize NdotL for directional light (uses calculated normal with normal map)
    // 8 = simplified diffuse (no specular, no normal map, no gamma/tone mapping)
    // 9 = visualize attenuation
    // 10 = visualize calculated normal (N from calculateNormal() - includes normal map)
    // 11 = visualize NdotL with calculated normal (N from calculateNormal())
    // ============================================================
    
    if (debugMode == 1) {
        // Debug mode 1: Visualize normals (fragNormal - geometric normal only)
        // Different colors on different faces are NORMAL for a cube (each face has different normal direction)
        // Should show smooth color transitions WITHIN each face if normals are correct
        // If you see diagonal patterns or half-circles WITHIN a face, normals are not properly transformed to world-space
        color = normalize(fragNormal) * 0.5 + 0.5;
    }
    else if (debugMode == 2) {
        // Debug mode 2: Visualize normal length
        // Should be ~1.0 everywhere (white) if normals are unit length after transformation
        float normalLen = length(fragNormal);
        // Green if length is correct (~1.0), red if too short, blue if too long
        if (normalLen > 0.99 && normalLen < 1.01) {
            color = vec3(0.0, 1.0, 0.0); // Green - correct
        } else if (normalLen < 0.01) {
            color = vec3(1.0, 0.0, 0.0); // Red - zero normal (error)
        } else {
            color = vec3(0.0, 0.0, 1.0); // Blue - incorrect length
        }
        // Also show as grayscale for magnitude visualization
        color = vec3(normalLen);
    }
    else if (debugMode == 3) {
        // Debug mode 3: Visualize tangents
        // Should show smooth transitions, no diagonal breaks
        color = normalize(fragTangent) * 0.5 + 0.5;
    }
    else if (debugMode == 4) {
        // Debug mode 4: Visualize bitangents
        // Should show smooth transitions, no diagonal breaks
        color = normalize(fragBitangent) * 0.5 + 0.5;
    }
    else if (debugMode == 5) {
        // Debug mode 5: Visualize TBN orthogonality
        // Check if normal and tangent are orthogonal (should be white everywhere)
        float dotNT = dot(fragNormal, fragTangent);
        float dotNB = dot(fragNormal, fragBitangent);
        float dotTB = dot(fragTangent, fragBitangent);
        // White if orthogonal (dot product ~0), colored if not
        float error = max(abs(dotNT), max(abs(dotNB), abs(dotTB)));
        if (error < 0.01) {
            color = vec3(1.0); // White - orthogonal
        } else {
            color = vec3(error * 10.0, 0.0, 0.0); // Red - not orthogonal
        }
    }
    else if (debugMode == 6) {
        // Debug mode 6: Visualize NdotL for point light (geometric normal only)
        // Should show circular pattern, not diagonal cut
        // Uses ONLY geometric normal (fragNormal) - same as mode 8
        // If you see diagonal cuts here but mode 8 works, there's a difference in calculation
        vec3 N_geom = normalize(fragNormal); // Geometric normal only (no normal map, no TBN)
        if (lightCount > 0) {
            vec3 lightPos = pointPositions[0].xyz;
            // Light direction: from fragment to light (same as mode 8 and normal rendering)
            vec3 L = normalize(lightPos - fragWorldPos);
            float debugNdotL = dot(N_geom, L);
            // Visualize NdotL directly (should be circular for point light)
            color = vec3(max(debugNdotL, 0.0));
        } else {
            // Fallback to directional light if no point lights
            vec3 L_dir = normalize(-lightDirection.xyz);
            float debugNdotL = dot(N_geom, L_dir);
            color = vec3(max(debugNdotL, 0.0));
        }
    }
    else if (debugMode == 7) {
        // Debug mode 7: Visualize NdotL for directional light (uses calculated normal with normal map)
        // Compare with mode 6 to see if normal map causes issues
        vec3 L_dir = normalize(-lightDirection.xyz);
        float debugNdotL = dot(N, L_dir); // N from calculateNormal() - may include normal map
        color = vec3(max(debugNdotL, 0.0));
    }
    else if (debugMode == 10) {
        // Debug mode 10: Visualize calculated normal (N from calculateNormal() - includes normal map)
        // Compare with mode 1 (geometric normal) to see differences
        color = normalize(N) * 0.5 + 0.5;
    }
    else if (debugMode == 11) {
        // Debug mode 11: Visualize NdotL with calculated normal (includes normal map)
        // Compare with mode 6 (geometric normal) to see if normal map causes diagonal cuts
        if (lightCount > 0) {
            vec3 lightPos = pointPositions[0].xyz;
            vec3 L = normalize(lightPos - fragWorldPos);
            float debugNdotL = dot(N, L); // N from calculateNormal() - may include normal map
            color = vec3(max(debugNdotL, 0.0));
        } else {
            vec3 L_dir = normalize(-lightDirection.xyz);
            float debugNdotL = dot(N, L_dir);
            color = vec3(max(debugNdotL, 0.0));
        }
    }
    else if (debugMode == 8) {
        // Debug mode 8: Simplified diffuse (no specular, no normal map, no gamma/tone mapping)
        // Isolates the problem to basic geometry/normals
        // IMPORTANT: This mode uses ONLY point lights, ignores directional light and ambient
        vec3 N_simple = normalize(fragNormal); // Use geometric normal only (no normal map)
        if (lightCount > 0) {
            vec3 lightPos = pointPositions[0].xyz;
            vec3 L = normalize(lightPos - fragWorldPos);
            float NdotL = max(dot(N_simple, L), 0.0);
            float dist = length(lightPos - fragWorldPos);
            float attenuation = 1.0 / (dist * dist);
            attenuation *= smoothstep(pointOuterRadii[0], pointInnerRadii[0], dist);
            color = albedo.rgb * pointColors[0].rgb * pointIntensities[0] * NdotL * attenuation;
        } else {
            // No point lights - show black (ignore directional light and ambient in this debug mode)
            // If you see lighting without point lights, it means directional light or ambient is active
            color = vec3(0.0);
        }
        // No ambient, no directional light, no specular, no tone mapping, no gamma correction
    }
    else if (debugMode == 9) {
        // Debug mode 9: Visualize attenuation
        // Should show smooth falloff, no sudden cuts
        if (lightCount > 0) {
            vec3 lightPos = pointPositions[0].xyz;
            float dist = length(lightPos - fragWorldPos);
            float attenuation = 1.0 / (dist * dist);
            attenuation *= smoothstep(pointOuterRadii[0], pointInnerRadii[0], dist);
            color = vec3(attenuation);
        } else {
            color = vec3(0.0);
        }
    }
    
    // Apply tone mapping and gamma correction only in normal mode (debugMode == 0)
    // Debug modes 1-9 skip post-processing for accurate visualization
    if (debugMode == 0) {
        // HDR tone mapping
        color = color / (color + vec3(1.0));
        
        // Gamma correction
        color = pow(color, vec3(1.0/2.2));
    }
    
    outColor = vec4(color, albedo.a);
}
