#pragma once 


#include <GL/glew.h>


class OpenGLFramebuffer
{
	GLuint fbo;
	GLuint texture;
	GLuint rbo;

	int m_width, m_height;
public:
	OpenGLFramebuffer(int width, int height);
	~OpenGLFramebuffer();

	void bind();
	void unbind();

	GLuint getTexture() const { return texture; }
	void resize(int newWidth, int newHeight);
private:
	void setupFramebuffer();
};