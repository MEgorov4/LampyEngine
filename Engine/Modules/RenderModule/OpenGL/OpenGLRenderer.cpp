#include "OpenGLRenderer.h"

#include <GL/glew.h>
#include <imgui.h>

#include "../../LoggerModule/Logger.h"
#include "../../ShaderCompilerModule/ShaderCompiler.h"
#include "../../ImGuiModule/OpenGLBackends/imgui_impl_opengl3.h"
#include "../../ImGuiModule/GLFWBackends/imgui_impl_glfw.h"

#include "OpenGLObjects/OpenGLFramebuffer.h"
#include "OpenGLObjects/OpenGLShader.h"
#include "OpenGLObjects/OpenGLVertexBuffer.h"
#include "OpenGLObjects/OpenGLMesh.h"

OpenGLRenderer::OpenGLRenderer(Window* window) : m_window(window)
{
	init();
}

void OpenGLRenderer::init()
{
	if (!glewInit())
	{
		LOG_INFO("OpenGLRenderer: Failed to initialize GLEW");
	}

	const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
	if (strstr(extensions, "GL_ARB_gl_spirv") == NULL || strstr(extensions, "GL_ARB_spirv_extensions") == NULL) {
		std::cerr << "SPIR-V не поддерживается!" << std::endl;
		exit(1);
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glfwSwapInterval(0);
	glViewport(0, 0, m_window->getExtent().width, m_window->getExtent().height);

	glClearColor(0, 0, 0, 1);

	m_offscreenFramebuffer = std::make_unique<OpenGLFramebuffer>(1920, 1080);

	initImGui();

	LOG_INFO("OpenGLRenderer: OpenGL initialized successfully");
}

void OpenGLRenderer::initImGui()
{
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(m_window->getGLFWWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 450");
}



void OpenGLRenderer::render()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0, 0, 0, 1.0f);
	glViewport(0, 0, m_window->getExtent().width, m_window->getExtent().height);

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(m_window->getGLFWWindow());

	m_offscreenFramebuffer->bind();
	renderWorld();
	m_offscreenFramebuffer->unbind();
}

void OpenGLRenderer::renderWorld()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);


	for (auto& renderObject : m_activeRenderObjects)
	{
		if (renderObject.shader)
		{
			renderObject.shader->use();

			glBindBufferBase(GL_UNIFORM_BUFFER, 0, dynamic_cast<OpenGLShader*>(renderObject.shader.get())->getUBO());

			ShaderUniformBlock block = { renderObject.modelMatrix, renderObject.viewMatrix, renderObject.projectionMatrix };
			renderObject.shader->setUniformBlock(block);

			if (renderObject.mesh)
			{
				renderObject.mesh->draw();
			}
		}
	}

	glDisable(GL_DEPTH_TEST);
}

void OpenGLRenderer::registerShader(const std::string& vertPath, const std::string& fragPath)
{

}

void OpenGLRenderer::removeShader(const std::string& vertPath, const std::string& fragPath)
{

}

void OpenGLRenderer::registerVertexData(const std::vector<Vertex>& vertexData, const std::string& pathToFile)
{
}

void OpenGLRenderer::removeVertexData(const std::vector<Vertex>& vertexData, const std::string& pathToFile)
{

}

void OpenGLRenderer::registerIndexData(const std::vector<uint32_t>& indexData, const std::string& pathToFile)
{

}

void OpenGLRenderer::removeIndexData(const std::vector<uint32_t>& indexData, const std::string& pathToFile)
{
}

void* OpenGLRenderer::getOffscreenImageDescriptor()
{
	return reinterpret_cast<void*>(static_cast<uintptr_t>(m_offscreenFramebuffer->getTexture()));
}

void OpenGLRenderer::waitIdle()
{
	glFinish();
}

