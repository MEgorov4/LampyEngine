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
	std::unique_ptr<OpenGLFramebuffer> m_textureFramebuffer;
	std::unique_ptr<OpenGLFramebuffer> m_customFramebuffer;
	
	std::unique_ptr<OpenGLMesh2D> m_quadMesh2D;

	GLuint m_quadVAO;
	GLuint m_quadVBO;

public:
	OpenGLRenderer(Window* window);

	void render() override;

	void* getOffscreenImageDescriptor() override;

	void waitIdle() override;

	virtual void drawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color) override;
private:
	void init();
	void initImGui();

	void debugMessageHandle(std::string& message);

	void renderWorld();

	void renderPass(const RenderPassData& renderPassData);

};