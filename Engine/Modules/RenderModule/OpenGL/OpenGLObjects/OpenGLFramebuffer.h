#pragma once

#include <GL/glew.h>

class OpenGLFramebuffer
{
private:
    GLuint fbo;
    int m_width, m_height;

    GLuint m_colorTexture;
    GLuint m_depthTexture;
    GLuint rbo;  

    bool m_depth;
public:
    OpenGLFramebuffer(int width, int height, bool useDepth = true);
    ~OpenGLFramebuffer();

    void bind();
    void unbind();
    void resize(int newWidth, int newHeight);

    void addColorAttachment(GLenum format = GL_RGBA8, GLenum type = GL_UNSIGNED_BYTE);
    void addDepthAttachment(bool asTexture = true);

    GLuint getColorTexture() const { return m_colorTexture; }
    GLuint getDepthTexture() const { return m_depthTexture; }

    GLuint getFBO() const {
        return fbo;
    }

};
