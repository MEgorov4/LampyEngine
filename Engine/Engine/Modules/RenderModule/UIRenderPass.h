#pragma once

namespace RenderModule::UIRenderPass
{
/// <summary>
/// Executes the UI rendering step using the configured UI backend and registered callbacks.
/// Intended to be called near the end of the frame, after the main scene rendering.
/// </summary>
void render();
} // namespace RenderModule::UIRenderPass


