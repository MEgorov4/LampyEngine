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

namespace RenderModule
{

	class TextureFactory
	{
		static std::unordered_map<std::string, std::weak_ptr<ITexture>> textureCache;
	public:
		static std::shared_ptr<ITexture> createOrGetTexture(const std::shared_ptr<ResourceModule::RTexture>& texture)
		{
			if (!textureCache[texture.get()->getGUID()].expired())
				return textureCache[texture.get()->getGUID()].lock();

			if (RC.getGraphicsAPI() == GraphicsAPI::OpenGL)
			{
				auto glTexture = std::make_shared<OpenGL::OpenGLTexture>(texture);
				textureCache[texture.get()->getGUID()] = glTexture;
				return glTexture;
			}

			return nullptr;
		}
	};

	std::unordered_map<std::string, std::weak_ptr<ITexture>> TextureFactory::textureCache;

	class MeshFactory
	{
		static std::unordered_map<std::string, std::weak_ptr<IMesh>> meshCache;
	public:
		static std::shared_ptr<IMesh> createOrGetMesh(const std::shared_ptr<ResourceModule::RMesh>& mesh)
		{
			if (!meshCache[mesh.get()->getGUID()].expired())
				return meshCache[mesh.get()->getGUID()].lock();

			if (RC.getGraphicsAPI() == GraphicsAPI::OpenGL)
			{
				auto glMesh = std::make_shared<OpenGL::OpenGLMesh>(mesh);
				meshCache[mesh.get()->getGUID()] = glMesh;
				return glMesh;
			}

			return nullptr;
		}
	};

	std::unordered_map<std::string, std::weak_ptr<IMesh>> MeshFactory::meshCache;


	class ShaderFactory
	{
		static std::unordered_map<std::string, std::weak_ptr<IShader>> shaderCache;
	public:
		static std::shared_ptr<IShader> createOrGetShader(const std::shared_ptr<ResourceModule::RShader>& vertShader, const std::shared_ptr<ResourceModule::RShader>& fragShader)
		{
			std::string hash = vertShader->getGUID() + fragShader->getGUID();
			if (!shaderCache[hash].expired())
				return shaderCache[hash].lock();

			if (RC.getGraphicsAPI() == GraphicsAPI::OpenGL)
			{
				auto glShader = std::make_shared<OpenGL::OpenGLShader>(vertShader, fragShader);
				shaderCache[hash] = glShader;
				return glShader;
			}
			return nullptr;
		}
	};

	std::unordered_map<std::string, std::weak_ptr<IShader>> ShaderFactory::shaderCache;
}
