#pragma once 
#include "../../Abstract/IMesh.h"
#include "../../../ResourceModule/Mesh.h"

namespace RenderModule::OpenGL
{
	class OpenGLMesh : public IMesh
	{
	public:
		OpenGLMesh(const std::shared_ptr<ResourceModule::RMesh>& mesh);
		virtual ~OpenGLMesh();

		unsigned int getVAO() { return m_VAO; }
		unsigned int getVBO() { return m_VBO; }
		unsigned int getIBO() { return m_IBO; }
		unsigned int getIndexCount() { return m_indexCount; }
	private:
		unsigned int m_VAO = 0;
		unsigned int m_VBO = 0;
		unsigned int m_IBO = 0;
		int m_indexCount = 0;

		void bind() const override;
		void draw() const override;
		void drawIndexed(int instanceCount) const override;
		void unbind() const override;
	};
}