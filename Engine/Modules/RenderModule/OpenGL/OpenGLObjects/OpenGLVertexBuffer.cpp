#include "OpenGLVertexBuffer.h"

#include "../../../ResourceModule/Mesh.h"

namespace RenderModule::OpenGL
{
	OpenGLVertexBuffer::OpenGLVertexBuffer(const std::vector<ResourceModule::MeshVertex>& vertices)
	{
		m_vertexCount = vertices.size();

		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);

		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(ResourceModule::MeshVertex), vertices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ResourceModule::MeshVertex), (void*)offsetof(ResourceModule::MeshVertex, pos));
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ResourceModule::MeshVertex), (void*)offsetof(ResourceModule::MeshVertex, uv));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ResourceModule::MeshVertex), (void*)offsetof(ResourceModule::MeshVertex, normal));
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		glDeleteBuffers(1, &m_VBO);
		glDeleteVertexArrays(1, &m_VAO);
	}

	void OpenGLVertexBuffer::bind() const
	{
		glBindVertexArray(m_VAO);
	}

	void OpenGLVertexBuffer::unbind() const
	{
		glBindVertexArray(0);
	}

	void OpenGLVertexBuffer::draw(GLenum mode) const
	{
		glDrawArrays(mode, 0, m_vertexCount);
	}
}
