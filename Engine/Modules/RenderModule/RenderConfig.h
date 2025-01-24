#pragma once
#include <cstdint>

enum class GraphicsAPI : uint8_t
{
	Vulkan,
	OpenGL
};
class RenderConfig
{
	uint32_t MAX_FRAMES_IN_FLIGHT = 2;
	GraphicsAPI GRAPHICS_API = GraphicsAPI::Vulkan;
	bool IMGUI_ENABLED = true;
public:
	static RenderConfig& getInstance()
	{
		static RenderConfig config;
		return config;
	}

	uint32_t getMaxFramesInFlight() const { return MAX_FRAMES_IN_FLIGHT; }
	GraphicsAPI getGraphicsAPI() const { return GRAPHICS_API; }
	bool getImGuiEnabled() const { return IMGUI_ENABLED; }

	void setMaxFramesInFlight(const uint32_t framesInFlight) { MAX_FRAMES_IN_FLIGHT = framesInFlight; };
	void setGraphicsAPI(GraphicsAPI graphicsAPI) { GRAPHICS_API = graphicsAPI; }
	void setImGuiEnabled(bool state) { IMGUI_ENABLED = state; }
private:
	RenderConfig() = default;
	~RenderConfig() = default;

	RenderConfig(const RenderConfig&) = delete;
	RenderConfig& operator=(const RenderConfig&) = delete;
};