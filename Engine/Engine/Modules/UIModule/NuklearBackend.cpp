#include "NuklearBackend.h"

#include <Modules/RenderModule/Abstract/RenderState.h>
#include <Modules/WindowModule/WindowModule.h>
#include <Modules/WindowModule/Window.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_IMPLEMENTATION
#include <nuklear.h>

namespace
{
// Vertex format matching Nuklear's expectations
struct NKVertex
{
    float position[2];
    float uv[2];
    nk_byte col[4];
};

// Simple OpenGL shader helpers
static GLuint compileShader(GLenum type, const char* src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        GLint logLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        if (logLen > 1)
        {
            std::string log;
            log.resize(static_cast<size_t>(logLen));
            glGetShaderInfoLog(shader, logLen, nullptr, log.data());
            LT_LOGE("NuklearBackend", std::format("Shader compile error: {}", log));
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint createProgram(const char* vsSrc, const char* fsSrc)
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, vsSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSrc);
    if (!vs || !fs)
    {
        if (vs)
            glDeleteShader(vs);
        if (fs)
            glDeleteShader(fs);
        return 0;
    }

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint status = GL_FALSE;
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status != GL_TRUE)
    {
        GLint logLen = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLen);
        if (logLen > 1)
        {
            std::string log;
            log.resize(static_cast<size_t>(logLen));
            glGetProgramInfoLog(prog, logLen, nullptr, log.data());
            LT_LOGE("NuklearBackend", std::format("Program link error: {}", log));
        }
        glDeleteProgram(prog);
        prog = 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

static nk_draw_null_texture g_nkNullTexture;
} // namespace

namespace UIModule
{
NuklearBackend::NuklearBackend(WindowModule::WindowModule* windowModule) noexcept
    : m_windowModule(windowModule)
{
    LT_LOGI("NuklearBackend", "Construct");

    initNuklear();
    initGLResources();
}

NuklearBackend::~NuklearBackend()
{
    LT_LOGI("NuklearBackend", "Destruct");

    shutdownGLResources();
    shutdownNuklear();
}

void NuklearBackend::initNuklear()
{
    // Allocate context & atlas using Nuklear's default allocator.
    m_ctx   = static_cast<nk_context*>(std::calloc(1, sizeof(nk_context)));
    m_atlas = static_cast<nk_font_atlas*>(std::calloc(1, sizeof(nk_font_atlas)));

    if (!m_ctx || !m_atlas)
    {
        LT_LOGE("NuklearBackend", "Failed to allocate Nuklear context or atlas");
        return;
    }

    nk_init_default(m_ctx, nullptr);

    nk_font_atlas_init_default(m_atlas);
    nk_font_atlas_begin(m_atlas);

    const float fontSize = 16.0f;
    const nk_font* font  = nk_font_atlas_add_default(m_atlas, fontSize, nullptr);

    int texWidth = 0;
    int texHeight = 0;
    const void* image = nk_font_atlas_bake(m_atlas, &texWidth, &texHeight, NK_FONT_ATLAS_RGBA32);

    if (!image || texWidth == 0 || texHeight == 0)
    {
        LT_LOGE("NuklearBackend", "Failed to bake Nuklear font atlas");
        return;
    }

    // Create OpenGL texture for font atlas.
    glGenTextures(1, &m_fontTexture);
    glBindTexture(GL_TEXTURE_2D, m_fontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    nk_font_atlas_end(m_atlas, nk_handle_id(static_cast<int>(m_fontTexture)), &g_nkNullTexture);

    if (font)
    {
        nk_style_set_font(m_ctx, &font->handle);
    }
}

void NuklearBackend::shutdownNuklear() noexcept
{
    if (m_atlas)
    {
        nk_font_atlas_clear(m_atlas);
        std::free(m_atlas);
        m_atlas = nullptr;
    }

    if (m_ctx)
    {
        nk_free(m_ctx);
        std::free(m_ctx);
        m_ctx = nullptr;
    }

    if (m_fontTexture)
    {
        glDeleteTextures(1, &m_fontTexture);
        m_fontTexture = 0;
    }
}

void NuklearBackend::initGLResources()
{
    // Create shader program for Nuklear rendering.
    static const char* vsSrc = R"(
        #version 330 core
        layout(location = 0) in vec2 Position;
        layout(location = 1) in vec2 TexCoord;
        layout(location = 2) in vec4 Color;

        uniform mat4 uProj;

        out vec2 Frag_UV;
        out vec4 Frag_Color;

        void main()
        {
            Frag_UV    = TexCoord;
            Frag_Color = Color;
            gl_Position = uProj * vec4(Position.xy, 0.0, 1.0);
        }
    )";

    static const char* fsSrc = R"(
        #version 330 core
        in vec2 Frag_UV;
        in vec4 Frag_Color;

        uniform sampler2D uTexture;

        out vec4 Out_Color;

        void main()
        {
            Out_Color = Frag_Color * texture(uTexture, Frag_UV.st);
        }
    )";

    m_shaderProgram = createProgram(vsSrc, fsSrc);
    if (!m_shaderProgram)
    {
        LT_LOGE("NuklearBackend", "Failed to create Nuklear GL shader program");
        return;
    }

    m_uniformProj    = glGetUniformLocation(m_shaderProgram, "uProj");
    m_uniformTexture = glGetUniformLocation(m_shaderProgram, "uTexture");

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
}

void NuklearBackend::shutdownGLResources() noexcept
{
    if (m_vao)
    {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_vbo)
    {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_ebo)
    {
        glDeleteBuffers(1, &m_ebo);
        m_ebo = 0;
    }

    if (m_shaderProgram)
    {
        glDeleteProgram(m_shaderProgram);
        m_shaderProgram = 0;
    }
}

void NuklearBackend::beginFrame() noexcept
{
    if (!m_ctx)
        return;

    nk_input_begin(m_ctx);

    // Feed all queued SDL events into Nuklear.
    for (const SDL_Event& evt : m_eventQueue)
    {
        processQueuedEvent(evt);
    }
    m_eventQueue.clear();
}

void NuklearBackend::endFrame() noexcept
{
    if (!m_ctx)
        return;

    nk_input_end(m_ctx);
}

void NuklearBackend::render() noexcept
{
    if (!m_ctx || !m_shaderProgram || !m_vao || !m_vbo || !m_ebo)
        return;

    auto* windowModule = m_windowModule;
    if (!windowModule)
        return;

    auto* window = windowModule->getWindow();
    if (!window)
        return;

    auto [width, height] = window->getWindowSize();
    if (width <= 0 || height <= 0)
        return;

    // Prepare buffers for conversion
    nk_buffer cmds;
    nk_buffer vbuf;
    nk_buffer ibuf;

    nk_buffer_init_default(&cmds);
    nk_buffer_init_default(&vbuf);
    nk_buffer_init_default(&ibuf);

    static const nk_draw_vertex_layout_element layout[] = {
        {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(NKVertex, position)},
        {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(NKVertex, uv)},
        {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(NKVertex, col)},
        {NK_VERTEX_LAYOUT_END},
    };

    nk_convert_config config{};
    config.vertex_layout = layout;
    config.vertex_size   = sizeof(NKVertex);
    config.vertex_alignment = alignof(NKVertex);
    config.null          = g_nkNullTexture;
    config.circle_segment_count = 22;
    config.curve_segment_count  = 22;
    config.arc_segment_count    = 22;
    config.global_alpha         = 1.0f;
    config.shape_AA             = NK_ANTI_ALIASING_ON;
    config.line_AA              = NK_ANTI_ALIASING_ON;

    const nk_flags convertResult = nk_convert(m_ctx, &cmds, &vbuf, &ibuf, &config);
    if (convertResult < 0)
    {
        // Conversion failed, just clear and exit.
        nk_clear(m_ctx);
        nk_buffer_free(&cmds);
        nk_buffer_free(&vbuf);
        nk_buffer_free(&ibuf);
        return;
    }

    // Upload vertex/index data to GPU
    const void* vertices = nk_buffer_memory_const(&vbuf);
    const nk_draw_index* indices =
        static_cast<const nk_draw_index*>(nk_buffer_memory_const(&ibuf));
    const size_t vertexSizeBytes = nk_buffer_total(&vbuf);
    const size_t indexSizeBytes  = nk_buffer_total(&ibuf);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertexSizeBytes), vertices, GL_STREAM_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indexSizeBytes), indices, GL_STREAM_DRAW);

    const GLsizei stride = static_cast<GLsizei>(sizeof(NKVertex));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<const void*>(NK_OFFSETOF(NKVertex, position)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<const void*>(NK_OFFSETOF(NKVertex, uv)));
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride,
                          reinterpret_cast<const void*>(NK_OFFSETOF(NKVertex, col)));

    // Build orthographic projection matrix
    float proj[4][4] = {};
    proj[0][0] = 2.0f / static_cast<float>(width);
    proj[1][1] = -2.0f / static_cast<float>(height);
    proj[2][2] = -1.0f;
    proj[3][0] = -1.0f;
    proj[3][1] = 1.0f;
    proj[3][3] = 1.0f;

    using RenderModule::BlendFunc;
    using RenderModule::CullFace;
    using RenderModule::DepthFunc;
    using RenderModule::RenderState;

    auto state = RenderState::saveState();

    RenderState::enableDepthTest(false);
    RenderState::enableCullFace(false);
    RenderState::setDepthMask(false);
    RenderState::enableBlend(true);
    RenderState::setBlendFunc(BlendFunc::SrcAlpha, BlendFunc::OneMinusSrcAlpha);

    GLboolean scissorWasEnabled = glIsEnabled(GL_SCISSOR_TEST);
    glEnable(GL_SCISSOR_TEST);

    glUseProgram(m_shaderProgram);
    glUniformMatrix4fv(m_uniformProj, 1, GL_FALSE, &proj[0][0]);
    glUniform1i(m_uniformTexture, 0);

    glActiveTexture(GL_TEXTURE0);

    // Iterate draw commands
    const nk_draw_command* cmd;
    const nk_draw_index* offset = nullptr;

    nk_draw_foreach(cmd, m_ctx, &cmds)
    {
        if (!cmd->elem_count)
            continue;

        glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(cmd->texture.id));

        const int x = static_cast<int>(cmd->clip_rect.x);
        const int y = static_cast<int>(height - (cmd->clip_rect.y + cmd->clip_rect.h));
        const int w = static_cast<int>(cmd->clip_rect.w);
        const int h = static_cast<int>(cmd->clip_rect.h);
        glScissor(x, y, w, h);

        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(cmd->elem_count), GL_UNSIGNED_SHORT, offset);
        offset += cmd->elem_count;
    }

    if (!scissorWasEnabled)
    {
        glDisable(GL_SCISSOR_TEST);
    }

    RenderState::restoreState(state);

    nk_clear(m_ctx);

    nk_buffer_free(&cmds);
    nk_buffer_free(&vbuf);
    nk_buffer_free(&ibuf);
}

void NuklearBackend::processQueuedEvent(const SDL_Event& evt) noexcept
{
    if (!m_ctx)
        return;

    switch (evt.type)
    {
    case SDL_EVENT_MOUSE_MOTION:
        nk_input_motion(m_ctx, evt.motion.x, evt.motion.y);
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        nk_input_scroll(m_ctx,
                        nk_vec2(static_cast<float>(evt.wheel.x), static_cast<float>(evt.wheel.y)));
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP: {
        const int down = (evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN) ? 1 : 0;
        int x = evt.button.x;
        int y = evt.button.y;
        nk_buttons btn = NK_BUTTON_LEFT;
        if (evt.button.button == SDL_BUTTON_RIGHT)
            btn = NK_BUTTON_RIGHT;
        else if (evt.button.button == SDL_BUTTON_MIDDLE)
            btn = NK_BUTTON_MIDDLE;
        nk_input_button(m_ctx, btn, x, y, down);
        break;
    }
    case SDL_EVENT_TEXT_INPUT: {
        // SDL3 text input gives UTF-8 in a char buffer.
        const char* text = evt.text.text;
        if (text)
        {
            while (*text)
            {
                nk_glyph glyph;
                nk_rune codepoint = 0;
                int bytes = nk_utf_decode(text, &codepoint, 4);
                if (bytes <= 0)
                    break;

                nk_utf_encode(codepoint, glyph, NK_UTF_SIZE);
                nk_input_glyph(m_ctx, glyph);
                text += bytes;
            }
        }
        break;
    }
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
        const int down = (evt.type == SDL_EVENT_KEY_DOWN) ? 1 : 0;
        const SDL_KeyboardEvent& key = evt.key;
        const SDL_Scancode sc = key.scancode;

        if (sc == SDL_SCANCODE_BACKSPACE)
            nk_input_key(m_ctx, NK_KEY_BACKSPACE, down);
        else if (sc == SDL_SCANCODE_DELETE)
            nk_input_key(m_ctx, NK_KEY_DEL, down);
        else if (sc == SDL_SCANCODE_RETURN || sc == SDL_SCANCODE_RETURN2)
            nk_input_key(m_ctx, NK_KEY_ENTER, down);
        else if (sc == SDL_SCANCODE_TAB)
            nk_input_key(m_ctx, NK_KEY_TAB, down);
        else if (sc == SDL_SCANCODE_LEFT)
            nk_input_key(m_ctx, NK_KEY_LEFT, down);
        else if (sc == SDL_SCANCODE_RIGHT)
            nk_input_key(m_ctx, NK_KEY_RIGHT, down);
        else if (sc == SDL_SCANCODE_HOME)
            nk_input_key(m_ctx, NK_KEY_TEXT_LINE_START, down);
        else if (sc == SDL_SCANCODE_END)
            nk_input_key(m_ctx, NK_KEY_TEXT_LINE_END, down);
        else if (sc == SDL_SCANCODE_PAGEUP)
            nk_input_key(m_ctx, NK_KEY_SCROLL_UP, down);
        else if (sc == SDL_SCANCODE_PAGEDOWN)
            nk_input_key(m_ctx, NK_KEY_SCROLL_DOWN, down);

        const SDL_Keymod mod = SDL_GetModState();
        nk_input_key(m_ctx, NK_KEY_SHIFT, (mod & SDL_KMOD_SHIFT) ? 1 : 0);
        nk_input_key(m_ctx, NK_KEY_CTRL, (mod & SDL_KMOD_CTRL) ? 1 : 0);
        break;
    }
    default:
        break;
    }
}

void NuklearBackend::processSDLEvent(const SDL_Event& evt) noexcept
{
    // Events are queued and later translated inside beginFrame between nk_input_begin/end.
    m_eventQueue.push_back(evt);
}
} // namespace UIModule

