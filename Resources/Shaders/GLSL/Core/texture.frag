#version 450

layout(location = 0) out vec4 outColor;  
layout(location = 0) in vec2 fragUV;      

layout(binding = 0) uniform sampler2D albedoTexture;


void main() 
{
    vec4 textureColor = texture(albedoTexture, fragUV);

    if(textureColor.rgb == vec3(0.0))
    {
        textureColor = vec4(0.5, 0.5, 0.5, 1.0);
    }
    outColor = textureColor;
}
