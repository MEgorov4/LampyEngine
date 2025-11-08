#pragma once

#include <GL/glew.h>
#include "../../Abstract/IMesh.h"

namespace RenderModule::OpenGL
{
	class OpenGLMesh2D final : public IMesh {
	public:
		OpenGLMesh2D();
		~OpenGLMesh2D() override;

		void bind() const override;
		void draw() const override;
		void unbind() const override;
		void drawIndexed(int instanceCount) const override;

	private:
		GLuint m_VAO = 0;
		GLuint m_VBO = 0;
	};
}
