#include "OpenGLRenderer.h"

#include <GL/glew.h>
#include <imgui.h>
#include <glm/gtx/string_cast.hpp>
#include "../../LoggerModule/Logger.h"
#include "../../ShaderCompilerModule/ShaderCompiler.h"
#include "../../ImGuiModule/OpenGLBackends/imgui_impl_opengl3.h"
#include "../../ImGuiModule/GLFWBackends/imgui_impl_glfw.h"

#include "OpenGLObjects/OpenGLFramebuffer.h"
#include "OpenGLObjects/OpenGLShader.h"
#include "OpenGLObjects/OpenGLVertexBuffer.h"
#include "OpenGLObjects/OpenGLMesh.h"
#include "OpenGLObjects/OpenGLMesh2D.h"
#include "OpenGLObjects/OpenGLTexture.h"

OpenGLRenderer::OpenGLRenderer(Window* window) : m_window(window)
{
	init();
}

void OpenGLRenderer::init()
{
	if (glewInit() == GL_SUCCESS_NV)
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

	m_shadowFramebuffer = std::make_unique<OpenGLFramebuffer>(1920, 1080, true);
	m_reflectionFramebuffer = std::make_unique<OpenGLFramebuffer>(1920, 1080);
	m_lightFramebuffer = std::make_unique<OpenGLFramebuffer>(1920, 1080);
	m_customFramebuffer = std::make_unique<OpenGLFramebuffer>(1920, 1080);
	m_finalFramebuffer = std::make_unique<OpenGLFramebuffer>(1920, 1080);

	m_quadMesh2D = std::make_unique<OpenGLMesh2D>();
	initImGui();

	LOG_INFO("OpenGLRenderer: OpenGL initialized successfully");
}

void OpenGLRenderer::initImGui()
{
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(m_window->getGLFWWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 450");
}

void OpenGLRenderer::debugMessageHandle(std::string& message)
{

}

void OpenGLRenderer::render()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0, 0, 0, 1.0f);
	glViewport(0, 0, m_window->getExtent().width, m_window->getExtent().height);

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	renderWorld();

	glfwSwapBuffers(m_window->getGLFWWindow());
}

void OpenGLRenderer::renderWorld()
{
	glEnable(GL_DEPTH_TEST);

	m_shadowFramebuffer->bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	renderPass(m_activeRenderPipelineData.shadowPass);
	m_shadowFramebuffer->unbind();

	m_reflectionFramebuffer->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	renderPass(m_activeRenderPipelineData.reflectionPass);
	m_reflectionFramebuffer->unbind();


	m_lightFramebuffer->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	renderPass(m_activeRenderPipelineData.lightPass);
	m_lightFramebuffer->unbind();


	m_customFramebuffer->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	renderPass(m_activeRenderPipelineData.customPass);
	m_customFramebuffer->unbind();


	glDisable(GL_DEPTH_TEST);
	m_finalFramebuffer->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	renderPass(m_activeRenderPipelineData.finalPass);
	m_finalFramebuffer->unbind();
	glEnable(GL_DEPTH_TEST);

	glDisable(GL_DEPTH_TEST);
}

void OpenGLRenderer::renderPass(const RenderPassData& renderPassData)
{

	ShaderUniformBlock_CameraData cameraData =
	{ m_activeRenderPipelineData.viewMatrix
	, m_activeRenderPipelineData.projMatrix
	, m_activeRenderPipelineData.cameraPosition };

	std::unordered_map<std::string, GLuint> textures;

	if (renderPassData.renderPassType == FINAL)
	{
		textures["customMap"] = m_customFramebuffer->getColorTexture();
		textures["lightMap"] = m_lightFramebuffer->getDepthTexture();
		textures["objectAlbedo"] = m_albedoGeneric->getTextureID();
		textures["objectEmission"] = m_emissionGeneric->getTextureID();
		textures["reflectionMap"] = m_reflectionFramebuffer->getColorTexture();
		textures["shadowMap"] = m_shadowFramebuffer->getDepthTexture();
	}
	for (auto& [shader, meshes] : renderPassData.batches)
	{
		shader->use();

		if (renderPassData.renderPassType == FINAL)
		{
			shader->bindTextures(textures);
			if (renderPassData.renderPassType == FINAL)
			{
				m_quadMesh2D->draw();
			}
		}
		else
		{

			if (shader->hasUniformBlock("CameraData"))
			{
				shader->setUniformData("CameraData", &cameraData, sizeof(ShaderUniformBlock_CameraData));
			}
			if (shader->hasUniformBlock("DirectionalLightData"))
			{
				ShaderUniformBlock_DirectionalLightData dirLightData = 
				{m_activeRenderPipelineData.directionalLight.direction, m_activeRenderPipelineData.directionalLight.color};

				shader->setUniformData("DirectionalLightData", &dirLightData, sizeof(ShaderUniformBlock_DirectionalLightData));
			}
			for (auto& [mesh, objects] : meshes)
			{
				for (auto& object : objects)
				{
					if (shader->hasUniformBlock("ModelMatrices"))
					{
						ShaderUniformBlock_ModelData modelData{ object.modelMatrix };
						shader->setUniformData("ModelMatrices", &modelData, sizeof(ShaderUniformBlock_ModelData));
					}

					if (mesh)
					{
						mesh->draw();
					}
				}
			}
		}
	}

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
	return reinterpret_cast<void*>(static_cast<uintptr_t>(m_finalFramebuffer->getColorTexture()));
}

void OpenGLRenderer::waitIdle()
{
	glFinish();
}

