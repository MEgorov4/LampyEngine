#include "OpenGLMesh2D.h"
namespace RenderModule::OpenGL
{
OpenGLMesh2D::OpenGLMesh2D() : IMesh(nullptr)
{
    LT_LOGI("RenderModule::OpenGLMesh2D", "Construct");

    constexpr float vertices[] = {-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f,
                                  -1.0f, 1.0f,  0.0f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f};

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

    glBindVertexArray(0);
}

OpenGLMesh2D::~OpenGLMesh2D()
{
    LT_LOGI("RenderModule::OpenGLMesh2D", "Destruct");
    glDeleteBuffers(1, &m_VBO);
    glDeleteVertexArrays(1, &m_VAO);
}

void OpenGLMesh2D::bind() const
{
    glBindVertexArray(m_VAO);
}

void OpenGLMesh2D::draw() const
{
    bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    unbind();
}

void OpenGLMesh2D::unbind() const
{
    glBindVertexArray(0);
}

void OpenGLMesh2D::drawIndexed(int instanceCount) const
{
}
} // namespace RenderModule::OpenGL
