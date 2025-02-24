#pragma once 

#include <GL/glew.h>
#include "../../Abstract/ITexture.h"

class OpenGLTexture : public ITexture
{
public:
	OpenGLTexture(const std::shared_ptr<RTexture>& texture);
	virtual ~OpenGLTexture();

	void bind() const override;
	void unbind() const override;

	GLuint getTextureID() { return m_textureID; }
private:
	GLuint m_textureID;
};