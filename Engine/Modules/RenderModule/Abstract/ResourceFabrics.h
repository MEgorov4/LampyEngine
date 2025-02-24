#pragma once 
#include <memory>


#include "../RenderConfig.h"
#include "ITexture.h"
#include "IMesh.h"
#include "IShader.h"
#include "../OpenGL/OpenGLTexture.h"
#include "../OpenGL/OpenGLMesh.h"
#include "../OpenGL/OpenGLObjects/OpenGLShader.h"

class TextureFactory
{
	static std::shared_ptr<ITexture> createTexture(const std::shared_ptr<RTexture>& texture)
	{
		switch (RC.getGraphicsAPI())
		{
		case GraphicsAPI::OpenGL:
			return std::make_shared<OpenGLTexture>(texture);
			break;
		case GraphicsAPI::Vulkan:
			return nullptr; // Vulkan objects implementation
			break;
		}
	}
};

class MeshFactory
{
	static std::shared_ptr<IMesh> createTexture(const std::shared_ptr<RMesh>& mesh)
	{
		switch (RC.getGraphicsAPI())
		{
		case GraphicsAPI::OpenGL:
			return std::make_shared<OpenGLMesh>(mesh);
			break;
		case GraphicsAPI::Vulkan:
			return nullptr; // Vulkan objects implementation
			break;
		}
	}
};


class ShaderFactory
{
	static std::shared_ptr<IShader> createTexture(const std::shared_ptr<RShader>& vertShader, const std::shared_ptr<RShader>& fragShader)
	{
		switch (RC.getGraphicsAPI())
		{
		case GraphicsAPI::OpenGL:
			return std::make_shared<OpenGLShader>(vertShader, fragShader);
			break;
		case GraphicsAPI::Vulkan:
			return nullptr; // Vulkan objects implementation
			break;
		}
	}
};
