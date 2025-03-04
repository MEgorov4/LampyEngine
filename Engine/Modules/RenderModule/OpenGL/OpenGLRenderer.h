#pragma once 

#include <memory>

#include "../IRenderer.h"
#include "../../WindowModule/Window.h"

class OpenGLFramebuffer;
class OpenGLShader;
class OpenGLVertexBuffer;
class OpenGLMesh2D;

class OpenGLRenderer : public IRenderer
{
	Window* m_window;

	std::unique_ptr<OpenGLFramebuffer> m_offscreenFramebuffer;
	std::unique_ptr<OpenGLFramebuffer> m_shadowFramebuffer;
	std::unique_ptr<OpenGLFramebuffer> m_reflectionFramebuffer;
	std::unique_ptr<OpenGLFramebuffer> m_lightFramebuffer;
	std::unique_ptr<OpenGLFramebuffer> m_finalFramebuffer;
	std::unique_ptr<OpenGLFramebuffer> m_customFramebuffer;
	
	std::unique_ptr<OpenGLMesh2D> m_quadMesh2D;

	GLuint m_quadVAO;
	GLuint m_quadVBO;
public:
	OpenGLRenderer(Window* window);

	void render() override;

	void registerShader(const std::string& vertPath, const std::string& fragPath) override;

	void removeShader(const std::string& vertPath, const std::string& fragPath) override;

	void registerVertexData(const std::vector<Vertex>& vertexData, const std::string& pathToFile) override;

	void removeVertexData(const std::vector<Vertex>& vertexData, const std::string& pathToFile) override;

	void registerIndexData(const std::vector<uint32_t>& indexData, const std::string& pathToFile) override;

	void removeIndexData(const std::vector<uint32_t>& indexData, const std::string& pathToFile) override;

	void* getOffscreenImageDescriptor() override;

	void waitIdle() override;

private:
	void init();
	void initImGui();

	void debugMessageHandle(std::string& message);

	void renderWorld();

	void renderPass(const RenderPassData& renderPassData);

};