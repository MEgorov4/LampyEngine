#pragma once 
#include <memory>
#include <unordered_map>

#include "../RenderConfig.h"
#include "ITexture.h"
#include "IMesh.h"
#include "IShader.h"
#include "IFramebuffer.h"

#include "../OpenGL/OpenGLObjects/OpenGLTexture.h"
#include "../OpenGL/OpenGLObjects/OpenGLMesh.h"
#include "../OpenGL/OpenGLObjects/OpenGLShader.h"
#include "../OpengL/OpenGLObjects/OpenGLFramebuffer.h"


namespace RenderModule
{
	class TextureFactory
	{
		static std::unordered_map<std::string, std::weak_ptr<ITexture>> textureCache;
	public:
		static std::shared_ptr<ITexture> createOrGetTexture(const std::shared_ptr<ResourceModule::RTexture>& texture)
		{
			if (!textureCache[texture->getGUID()].expired())
				return textureCache[texture->getGUID()].lock();

			if (RC.getGraphicsAPI() == GraphicsAPI::OpenGL)
			{
				auto glTexture = std::make_shared<OpenGL::OpenGLTexture>(texture);
				textureCache[texture->getGUID()] = glTexture;
				return glTexture;
			}

			return nullptr;
		}
	};


	class MeshFactory
	{
		static std::unordered_map<std::string, std::weak_ptr<IMesh>> meshCache;
	public:
		static std::shared_ptr<IMesh> createOrGetMesh(const std::shared_ptr<ResourceModule::RMesh>& mesh)
		{
			if (!meshCache[mesh->getGUID()].expired())
				return meshCache[mesh->getGUID()].lock();

			if (RC.getGraphicsAPI() == GraphicsAPI::OpenGL)
			{
				auto glMesh = std::make_shared<OpenGL::OpenGLMesh>(mesh);
				meshCache[mesh->getGUID()] = glMesh;
				return glMesh;
			}

			return nullptr;
		}
	};



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


	class FramebufferFactory
	{
		static std::unordered_map<std::string, std::weak_ptr<IFramebuffer>> framebufferCache;
	public:
		static std::shared_ptr<IFramebuffer> createOrGetFramebuffer(const FramebufferData& data)
		{
			std::string hash = data.name;
			if (!framebufferCache[hash].expired())
				return framebufferCache[hash].lock();

			if (RC.getGraphicsAPI() == GraphicsAPI::OpenGL)
			{
				auto glFramebuffer = std::make_shared<OpenGL::OpenGLFramebuffer>(data);
				framebufferCache[hash] = glFramebuffer;
				return glFramebuffer;
			}
			return nullptr;
		}
	};

}
