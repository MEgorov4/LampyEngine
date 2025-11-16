#include "UISystem.h"

#include "NuklearBackend.h"

#include <nuklear.h>

namespace UIModule
{
void UISystem::update(float dt)
{
    (void)dt;
    if (!m_backend)
        return;

    nk_context* ctx = m_backend->ctx();
    if (!ctx)
        return;

    // Simple HUD window in the top-left corner.
    if (nk_begin(ctx, "HUD", nk_rect(20, 20, 260, 90), NK_WINDOW_NO_SCROLLBAR))
    {
        nk_layout_row_dynamic(ctx, 24, 1);
        nk_label(ctx, "Nuklear UI backend active", NK_TEXT_LEFT);
        nk_label(ctx, "Use this area for engine stats / debug info", NK_TEXT_LEFT);
    }
    nk_end(ctx);

    // Test controls window (draggable).
    if (nk_begin(ctx, "UI Test", nk_rect(300, 40, 260, 180),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE))
    {
        static float value = 0.5f;
        static int counter = 0;

        nk_layout_row_dynamic(ctx, 28, 1);
        nk_label(ctx, "Test panel", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 24, 1);
        nk_label(ctx, "Slider:", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 24, 1);
        nk_slider_float(ctx, 0.0f, &value, 1.0f, 0.01f);

        nk_layout_row_dynamic(ctx, 30, 1);
        if (nk_button_label(ctx, "Click me"))
        {
            ++counter;
        }

        nk_layout_row_dynamic(ctx, 24, 1);
        char buf[64];
        std::snprintf(buf, sizeof(buf), "Clicked %d times", counter);
        nk_label(ctx, buf, NK_TEXT_LEFT);
    }
    nk_end(ctx);
}
} // namespace UIModule

