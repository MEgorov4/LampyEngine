#pragma once
#include <cstdint>

enum class GraphicsAPI : uint32_t
{
	Vulkan,
	OpenGL
};
class RenderConfig
{
	uint32_t MAX_FRAMES_IN_FLIGHT = 2;
	GraphicsAPI GRAPHICS_API = GraphicsAPI::Vulkan;
public:
	static RenderConfig& getInstance()
	{
		static RenderConfig config;
		return config;
	}

	uint32_t getMaxFramesInFlight() const { return MAX_FRAMES_IN_FLIGHT; }
	GraphicsAPI getGraphicsAPI() const { return GRAPHICS_API; }

	void setMaxFramesInFlight(const uint32_t framesInFlight) { MAX_FRAMES_IN_FLIGHT = framesInFlight; };
	void setGraphicsAPI(GraphicsAPI graphicsAPI) { GRAPHICS_API = graphicsAPI; }
private:
	RenderConfig() = default;
	~RenderConfig() = default;

	RenderConfig(const RenderConfig&) = delete;
	RenderConfig& operator=(const RenderConfig&) = delete;
};