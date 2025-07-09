#include "RenderResourcesFactory.h"

namespace RenderModule
{
	std::unordered_map<std::string, std::weak_ptr<ITexture>> TextureFactory::textureCache;
	std::unordered_map<std::string, std::weak_ptr<IMesh>> MeshFactory::meshCache;
	std::unordered_map<std::string, std::weak_ptr<IShader>> ShaderFactory::shaderCache;
	std::unordered_map<std::string, std::weak_ptr<IFramebuffer>> FramebufferFactory::framebufferCache;
}