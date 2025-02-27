#pragma once 
#include <GL/glew.h>
#include "../../Abstract/IMesh.h"
#include "../../../ResourceModule/Mesh.h"


class OpenGLMesh : public IMesh
{
public:
	OpenGLMesh(const std::shared_ptr<RMesh>& mesh);
	virtual ~OpenGLMesh();

	GLuint getVAO() { return m_VAO; }
	GLuint getVBO() { return m_VBO; }
	GLuint getIBO() { return m_IBO; }
	GLuint getIndexCount() { return m_indexCount; }
private:
	GLuint m_VAO = 0;
	GLuint m_VBO = 0;
	GLuint m_IBO = 0;
	GLsizei m_indexCount = 0;

	void bind() const override;
	void draw() const override;
	void unbind() const override;
};