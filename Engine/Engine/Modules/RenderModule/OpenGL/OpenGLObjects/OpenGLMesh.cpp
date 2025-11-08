#include "OpenGLMesh.h"
#include <GL/glew.h>

namespace RenderModule::OpenGL
{
OpenGLMesh::OpenGLMesh(const std::shared_ptr<ResourceModule::RMesh> &mesh)
    : IMesh(mesh), m_indexCount(mesh->getMeshData().indices.size())
{
    LT_ASSERT_MSG(mesh, "Mesh resource is null");
    
    LT_LOGI("RenderModule::OpenGLMesh", "Contruct");
    std::vector<uint32_t, ProfileAllocator<uint32_t>> indices = mesh->getMeshData().indices;
    std::vector<ResourceModule::MeshVertex, ProfileAllocator<ResourceModule::MeshVertex>> vertices =
        mesh->getMeshData().vertices;

    LT_ASSERT_MSG(!vertices.empty(), "Mesh has no vertices");
    LT_ASSERT_MSG(!indices.empty(), "Mesh has no indices");

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(ResourceModule::MeshVertex), vertices.data(),
                 GL_STATIC_DRAW);

    glGenBuffers(1, &m_IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ResourceModule::MeshVertex),
                          (const void *)offsetof(ResourceModule::MeshVertex, pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ResourceModule::MeshVertex),
                          (const void *)offsetof(ResourceModule::MeshVertex, uv));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ResourceModule::MeshVertex),
                          (const void *)offsetof(ResourceModule::MeshVertex, normal));

    glBindVertexArray(0);
}

OpenGLMesh::~OpenGLMesh()
{
    LT_LOGI("RenderModule::OpenGLMesh", "Destruct");
    glDeleteBuffers(1, &m_IBO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteVertexArrays(1, &m_VAO);
}

void OpenGLMesh::bind() const
{
    glBindVertexArray(m_VAO);
}

void OpenGLMesh::draw() const
{
    LT_ASSERT_MSG(m_indexCount > 0, "Cannot draw mesh with zero indices");
    bind();
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
    unbind();
}

void OpenGLMesh::drawIndexed(GLsizei instanceCount) const
{
    bind();
    glDrawElementsInstanced(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0, instanceCount);
    unbind();
}

void OpenGLMesh::unbind() const
{
    glBindVertexArray(0);
}
} // namespace RenderModule::OpenGL
