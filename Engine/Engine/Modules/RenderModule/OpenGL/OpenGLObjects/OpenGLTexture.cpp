#include "OpenGLTexture.h"

#include <GL/glew.h>
#include <stdexcept>

namespace RenderModule::OpenGL
{
OpenGLTexture::OpenGLTexture(const std::shared_ptr<ResourceModule::RTexture> &texture) : ITexture(texture)
{
    LT_LOGI("RenderModule::OpenGLTexture", "Construct");
    
    if (!texture)
    {
        LT_LOGE("OpenGLTexture", "Texture resource is null");
        throw std::runtime_error("Texture resource is null");
    }
    
    const auto& info = texture->getInfo();
    if (info.width == 0 || info.height == 0)
    {
        LT_LOGE("OpenGLTexture", "Invalid texture dimensions: " + std::to_string(info.width) + "x" + std::to_string(info.height));
        throw std::runtime_error("Invalid texture dimensions");
    }
    
    if (info.pixels.empty())
    {
        LT_LOGE("OpenGLTexture", "Texture pixels data is empty");
        throw std::runtime_error("Texture pixels data is empty");
    }
    
    glGenTextures(1, &m_textureID);

    glBindTexture(GL_TEXTURE_2D, m_textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, info.width, info.height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, info.pixels.data());

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

OpenGLTexture::~OpenGLTexture()
{
    LT_LOGI("RenderModule::OpenGLTexture", "Destruct");
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
} // namespace RenderModule::OpenGL
