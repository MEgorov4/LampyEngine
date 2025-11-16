#pragma once

#include <EngineMinimal.h>
#include <Modules/RenderModule/IUIRenderBackend.h>

#include <SDL3/SDL.h>
#include <GL/glew.h>

#include <vector>

struct nk_context;
struct nk_font_atlas;

namespace WindowModule
{
class WindowModule;
}

namespace UIModule
{
/// <summary>
/// Nuklear-based implementation of IUIRenderBackend.
/// For now provides a minimal stub; full rendering will be implemented subsequently.
/// </summary>
class NuklearBackend final : public RenderModule::IUIRenderBackend
{
    WindowModule::WindowModule* m_windowModule = nullptr;

    // Nuklear core objects
    nk_context* m_ctx      = nullptr;
    nk_font_atlas* m_atlas = nullptr;

    // OpenGL resources for Nuklear rendering
    GLuint m_vao            = 0;
    GLuint m_vbo            = 0;
    GLuint m_ebo            = 0;
    GLuint m_fontTexture    = 0;
    GLuint m_shaderProgram  = 0;
    GLint  m_uniformProj    = -1;
    GLint  m_uniformTexture = -1;

    // Deferred SDL events processed at the beginning of each UI frame.
    std::vector<SDL_Event> m_eventQueue;

    void initNuklear();
    void shutdownNuklear() noexcept;
    void initGLResources();
    void shutdownGLResources() noexcept;

    void processQueuedEvent(const SDL_Event& evt) noexcept;

  public:
    explicit NuklearBackend(WindowModule::WindowModule* windowModule) noexcept;
    ~NuklearBackend() override;

    void beginFrame() noexcept override;
    void endFrame() noexcept override;
    void render() noexcept override;
    void processSDLEvent(const SDL_Event& evt) noexcept override;

    /// <summary>
    /// Returns a valid Nuklear context pointer after successful UIModule startup.
    /// </summary>
    nk_context* ctx() noexcept { return m_ctx; }
};
} // namespace UIModule
