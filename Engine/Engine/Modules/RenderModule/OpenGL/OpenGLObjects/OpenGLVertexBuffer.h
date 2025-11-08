#pragma once

#include "Foundation/Profiler/ProfileAllocator.h"

#include <EngineMinimal.h>
#include <GL/glew.h>
#include <vector>
namespace ResourceModule
{
class MeshVertex;
}
namespace RenderModule::OpenGL
{
class OpenGLVertexBuffer
{
  public:
    OpenGLVertexBuffer(
        const std::vector<ResourceModule::MeshVertex, ProfileAllocator<ResourceModule::MeshVertex>>& vertices);
    ~OpenGLVertexBuffer();

    void bind() const;
    void unbind() const;
    void draw(GLenum mode = GL_TRIANGLES) const;

  private:
    GLuint m_VAO, m_VBO;
    GLsizei m_vertexCount;
};
} // namespace RenderModule::OpenGL
