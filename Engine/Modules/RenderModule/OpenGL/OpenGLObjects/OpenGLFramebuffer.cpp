#include "OpenGLFramebuffer.h"
#include "../../../LoggerModule/Logger.h"

namespace RenderModule::OpenGL
{
	OpenGLFramebuffer::OpenGLFramebuffer(int width, int height, bool useDepth)
		: m_width(width), m_height(height), m_depth(useDepth), fbo(0),
		m_colorTexture(0), m_depthTexture(0), rbo(0)
	{
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		addColorAttachment();

		if (useDepth)
			addDepthAttachment(true);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			//LOG_ERROR("OpenGLFramebuffer: Framebuffer is not complete!");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		glDeleteFramebuffers(1, &fbo);
		glDeleteTextures(1, &m_colorTexture);
		glDeleteTextures(1, &m_depthTexture);
		glDeleteRenderbuffers(1, &rbo);
	}

	void OpenGLFramebuffer::bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glViewport(0, 0, m_width, m_height);
	}

	void OpenGLFramebuffer::unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::resize(int newWidth, int newHeight)
	{
		if (m_width == newWidth && m_height == newHeight)
			return;

		m_width = newWidth;
		m_height = newHeight;

		glDeleteTextures(1, &m_colorTexture);
		glDeleteTextures(1, &m_depthTexture);
		glDeleteRenderbuffers(1, &rbo);

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		addColorAttachment();
		addDepthAttachment(true);
		glViewport(0, 0, newWidth, newHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::addColorAttachment(GLenum format, GLenum type)
	{
		glGenTextures(1, &m_colorTexture);
		glBindTexture(GL_TEXTURE_2D, m_colorTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, GL_RGBA, type, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);

		GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawBuffers);
	}

	void OpenGLFramebuffer::addDepthAttachment(bool asTexture)
	{
		if (asTexture)
		{
			// Используем текстуру для глубины
			glGenTextures(1, &m_depthTexture);
			glBindTexture(GL_TEXTURE_2D, m_depthTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);
		}
		else
		{
			// Используем Renderbuffer (лучше для производительности, но нельзя сэмплировать)
			glGenRenderbuffers(1, &rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_width, m_height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
		}
	}
}
