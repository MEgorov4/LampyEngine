#pragma once 
#include <memory>
#include <unordered_map>

#include "../RenderConfig.h"
#include "ITexture.h"
#include "IMesh.h"
#include "IShader.h"
#include "../OpenGL/OpenGLObjects/OpenGLTexture.h"
#include "../OpenGL/OpenGLObjects/OpenGLMesh.h"
#include "../OpenGL/OpenGLObjects/OpenGLShader.h"


class TextureFactory
{
	static std::unordered_map<std::string, std::weak_ptr<ITexture>> textureCache;
public:
	static std::shared_ptr<ITexture> createOrGetTexture(const std::shared_ptr<RTexture>& texture)
	{
		if (!textureCache[texture.get()->getGUID()].expired())
			return textureCache[texture.get()->getGUID()].lock();

		if (RC.getGraphicsAPI() == GraphicsAPI::OpenGL)
		{
			auto glTexture = std::make_shared<OpenGLTexture>(texture);
			textureCache[texture.get()->getGUID()] = glTexture;
			return glTexture;
		}
		else if (RC.getGraphicsAPI() == GraphicsAPI::Vulkan)
		{
			return nullptr;
		}

		return nullptr;
	}
};

std::unordered_map<std::string, std::weak_ptr<ITexture>> TextureFactory::textureCache;

class MeshFactory
{
	static std::unordered_map<std::string, std::weak_ptr<IMesh>> meshCache;
public:
	static std::shared_ptr<IMesh> createOrGetMesh(const std::shared_ptr<RMesh>& mesh)
	{
		if (!meshCache[mesh.get()->getGUID()].expired())
			return meshCache[mesh.get()->getGUID()].lock();

		if (RC.getGraphicsAPI() == GraphicsAPI::OpenGL)
		{
			auto glMesh = std::make_shared<OpenGLMesh>(mesh);
			meshCache[mesh.get()->getGUID()] = glMesh;
			return glMesh;
		}
		else if (RC.getGraphicsAPI() == GraphicsAPI::Vulkan)
		{
			return nullptr;
		}

		return nullptr;
	}
};

std::unordered_map<std::string, std::weak_ptr<IMesh>> MeshFactory::meshCache;


class ShaderFactory
{
	static std::unordered_map<std::string, std::weak_ptr<IShader>> shaderCache;
public:
	static std::shared_ptr<IShader> createOrGetShader(const std::shared_ptr<RShader>& vertShader, const std::shared_ptr<RShader>& fragShader)
	{
		std::string hash = vertShader.get()->getGUID() + fragShader.get()->getGUID();
		if (!shaderCache[hash].expired())
			return shaderCache[hash].lock();

		if (RC.getGraphicsAPI() == GraphicsAPI::OpenGL)
		{
			auto glShader = std::make_shared<OpenGLShader>(vertShader, fragShader);
			shaderCache[hash] = glShader;
			return glShader;
		}
		else if (RC.getGraphicsAPI() == GraphicsAPI::Vulkan)
		{
			return nullptr;
		}

		return nullptr;
	}
};

std::unordered_map<std::string, std::weak_ptr<IShader>> ShaderFactory::shaderCache;
