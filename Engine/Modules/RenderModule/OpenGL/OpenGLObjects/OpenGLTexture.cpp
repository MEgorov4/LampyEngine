#include "OpenGLTexture.h"
namespace RenderModule::OpenGL
{


	OpenGLTexture::OpenGLTexture(const std::shared_ptr<ResourceModule::RTexture>& texture) : ITexture(texture)
	{
		glGenTextures(1, &m_textureID);

		glBindTexture(GL_TEXTURE_2D, m_textureID);

		glTexImage2D(GL_TEXTURE_2D
			, 0
			, GL_RGBA
			, texture->getTextureInfo().texWidth
			, texture->getTextureInfo().texHeight
			, 0
			, GL_RGBA
			, GL_UNSIGNED_BYTE
			, texture->getTextureInfo().pixels.data());

		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	OpenGLTexture::~OpenGLTexture()
	{
		glDeleteTextures(GL_TEXTURE_2D, &m_textureID);
	}

	void OpenGLTexture::bind() const
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
	}

	void OpenGLTexture::unbind() const
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}
