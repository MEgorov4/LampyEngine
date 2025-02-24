#pragma once 

#include <memory>

#include "../IRenderer.h"
#include "../../WindowModule/Window.h"

class OpenGLFramebuffer;
class OpenGLShader;
class OpenGLVertexBuffer;

class OpenGLRenderer : public IRenderer
{
	Window* m_window;
	std::unique_ptr<OpenGLFramebuffer> m_offscreenFramebuffer;
	std::unique_ptr<OpenGLShader> m_shader;

	std::vector <std::unique_ptr<OpenGLVertexBuffer>> m_vertexBuffers;
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

	void renderWorld();
};