#pragma once 

#include <GL/glew.h>
#include <vector>

#include "../../Vulkan/VulkanObjects/Vertex.h"

class OpenGLVertexBuffer
{
public:
	OpenGLVertexBuffer(const std::vector<Vertex>& vertices);
	~OpenGLVertexBuffer();

	void bind() const;
	void unbind() const;
	void draw(GLenum mode = GL_TRIANGLES) const;

private:
	GLuint m_VAO, m_VBO;
	GLsizei m_vertexCount;
};