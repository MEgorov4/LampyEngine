#include "OpenGLMesh.h"
#include "../../../LoggerModule/Logger.h"
namespace RenderModule::OpenGL
{
	OpenGLMesh::OpenGLMesh(const std::shared_ptr<ResourceModule::RMesh>& mesh) : IMesh(mesh), m_indexCount(mesh->getIndicesData().size())
	{
		std::vector<uint32_t> indices = mesh->getIndicesData();
		std::vector<ResourceModule::MeshVertex> vertices = mesh->getVertexData();

		glGenVertexArrays(1, &m_VAO);
		glBindVertexArray(m_VAO);

		glGenBuffers(1, &m_VBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(ResourceModule::MeshVertex), vertices.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &m_IBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ResourceModule::MeshVertex), (const void*)offsetof(ResourceModule::MeshVertex, pos));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ResourceModule::MeshVertex), (const void*)offsetof(ResourceModule::MeshVertex, uv));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ResourceModule::MeshVertex), (const void*)offsetof(ResourceModule::MeshVertex, normal));

		glBindVertexArray(0);
	}

	OpenGLMesh::~OpenGLMesh()
	{
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
}
