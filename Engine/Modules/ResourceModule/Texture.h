#pragma once

#include <string>
#include <vector>

#include "BaseResource.h"
#include "../MemoryModule/GarbageAllocator.h"

namespace ResourceModule
{
	struct TextureInfo
	{
		std::vector<uint8_t> pixels;
		int texWidth;
		int texHeight;
		int texChannels;
	};

	class RTexture : public BaseResource
	{
	public:
		RTexture(const std::string& path);
		~RTexture();

		const TextureInfo& getTextureInfo() const
		{
			return textureInfo;
		}
	private:
		TextureInfo textureInfo;
	};
}
