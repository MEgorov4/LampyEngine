#pragma once 
#include <memory>


#include "../RenderConfig.h"
#include "ITexture.h"
#include "IMesh.h"
#include "IShader.h"
#include "../OpenGL/OpenGLObjects/OpenGLTexture.h"
#include "../OpenGL/OpenGLObjects/OpenGLMesh.h"
#include "../OpenGL/OpenGLObjects/OpenGLShader.h"

class TextureFactory
{
public:
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
public:
	static std::shared_ptr<IMesh> createMesh(const std::shared_ptr<RMesh>& mesh)
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
public:
	static std::shared_ptr<IShader> createShader(const std::shared_ptr<RShader>& vertShader, const std::shared_ptr<RShader>& fragShader)
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
