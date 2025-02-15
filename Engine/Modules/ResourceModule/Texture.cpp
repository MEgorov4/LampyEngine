#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdexcept>

RTexture::RTexture(const std::string& path)
{
    int width, height, channels;
    stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!data) 
	{
        throw std::runtime_error("Failed to load texture: " + path);
    }

	textureInfo.texWidth = width;
	textureInfo.texHeight = height;
	textureInfo.texChannels = channels;
	textureInfo.pixels.assign(data, data + (width * height * 4));

	stbi_image_free(data);
}

RTexture::~RTexture()
{
	
}
