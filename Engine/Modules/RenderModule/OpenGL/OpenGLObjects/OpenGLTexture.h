#pragma once 

#include <GL/glew.h>
#include "../../Abstract/ITexture.h"
namespace RenderModule::OpenGL
{
	class OpenGLTexture : public ITexture
	{
	public:
		OpenGLTexture(const std::shared_ptr<ResourceModule::RTexture>& texture);
		virtual ~OpenGLTexture();

		void bind() const override;
		void unbind() const override;

		uint32_t getTextureID() override { return m_textureID; }
	private:
		GLuint m_textureID;
	};
}