#pragma once
#include <cstdint>

namespace RenderModule
{

	enum class GraphicsAPI
	{
		OpenGL
	};

	enum class RenderOutputMode
	{
		OffscreenTexture,
		WindowSwapchain
	};

	/// <summary>
	/// Singleton class that manages rendering configuration settings.
	/// </summary>
	class RenderConfig
	{
		GraphicsAPI GRAPHICS_API = GraphicsAPI::OpenGL;

        uint32_t MAX_FRAMES_IN_FLIGHT = 2; ///< Maximum number of frames that can be in flight at once.
        bool IMGUI_ENABLED = true; ///< Flag indicating whether ImGui is enabled.
        bool DEBUG_PASS_ENABLED = true; ///< Flag indicating whether debug pass rendering is enabled.
        bool GRID_PASS_ENABLED = true; ///< Flag indicating whether grid rendering is enabled.
        RenderOutputMode OUTPUT_MODE = RenderOutputMode::OffscreenTexture;

	public:
		/// <summary>
		/// Retrieves the singleton instance of the RenderConfig class.
		/// </summary>
		/// <returns>Reference to the RenderConfig instance.</returns>
		static RenderConfig& getInstance()
		{
			static RenderConfig config;
			return config;
		}

		/// <summary>
		/// Gets the maximum number of frames that can be in flight simultaneously.
		/// </summary>
		/// <returns>Maximum frames in flight.</returns>
		uint32_t getMaxFramesInFlight() const { return MAX_FRAMES_IN_FLIGHT; }
		GraphicsAPI getGraphicsAPI() const { return GRAPHICS_API; }
		RenderOutputMode getOutputMode() const { return OUTPUT_MODE; }
        /// <summary>
        /// Checks whether ImGui rendering is enabled.
        /// </summary>
        /// <returns>True if ImGui is enabled, otherwise false.</returns>
        bool getImGuiEnabled() const { return IMGUI_ENABLED; }
        /// <summary>
        /// Checks whether the debug render pass is enabled.
        /// </summary>
        /// <returns>True if the debug pass should run, otherwise false.</returns>
        bool getDebugPassEnabled() const { return DEBUG_PASS_ENABLED; }
        /// <summary>
        /// Checks whether the grid render pass is enabled.
        /// </summary>
        /// <returns>True if the grid pass should run, otherwise false.</returns>
        bool getGridPassEnabled() const { return GRID_PASS_ENABLED; }

		/// <summary>
		/// Sets the maximum number of frames in flight.
		/// </summary>
		/// <param name="framesInFlight">The new maximum frame count.</param>
		void setMaxFramesInFlight(const uint32_t framesInFlight) { MAX_FRAMES_IN_FLIGHT = framesInFlight; }

        /// <summary>
        /// Enables or disables ImGui rendering.
        /// </summary>
        /// <param name="state">True to enable ImGui, false to disable.</param>
        void setImGuiEnabled(bool state) { IMGUI_ENABLED = state; }
        /// <summary>
        /// Enables or disables the debug render pass.
        /// </summary>
        /// <param name="state">True to enable the debug pass, false to disable.</param>
        void setDebugPassEnabled(bool state) { DEBUG_PASS_ENABLED = state; }
        /// <summary>
        /// Enables or disables the grid render pass.
        /// </summary>
        /// <param name="state">True to enable the grid pass, false to disable.</param>
        void setGridPassEnabled(bool state) { GRID_PASS_ENABLED = state; }

		void setOutputMode(RenderOutputMode mode) { OUTPUT_MODE = mode; }

	private:
		/// <summary>
		/// Private constructor to enforce singleton pattern.
		/// </summary>
		RenderConfig() = default;

		/// <summary>
		/// Private destructor to prevent external deletion.
		/// </summary>
		~RenderConfig() = default;

		/// <summary>
		/// Deleted copy constructor to prevent duplication.
		/// </summary>
		RenderConfig(const RenderConfig&) = delete;

		/// <summary>
		/// Deleted assignment operator to prevent reassignment.
		/// </summary>
		RenderConfig& operator=(const RenderConfig&) = delete;
	};

	inline RenderConfig& RC = RenderConfig::getInstance();
}
