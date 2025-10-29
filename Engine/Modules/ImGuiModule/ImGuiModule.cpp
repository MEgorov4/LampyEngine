#include "ImGuiModule.h"

#include <imgui.h>

#include <Modules/InputModule/InputModule.h>
#include <Modules/WindowModule/WindowModule.h>
#include <Modules/WindowModule/Window.h>

#include <SDL3/SDL.h>
#include <GL/glew.h>
#include "OpenGLBackends/imgui_impl_opengl3.h"
#include "SDLBackends/imgui_impl_sdl3.h"
#include "GUIObject.h"

namespace ImGUIModule
{
    void ImGUIModule::startup()
    {
        m_inputModule = GCM(InputModule::InputModule);
        m_windowModule = GCM(WindowModule::WindowModule);
        
        m_inputModule->OnEvent.subscribe(std::bind(&ImGUIModule::onEvent, this, std::placeholders::_1));
        
        LT_LOGI("ImGuiModule", "Startup");

        ImGui::CreateContext();
        auto window = m_windowModule->getWindow();

        LT_LOGI("ImGuiModule", "Init dear ImGui");

        ImGui_ImplSDL3_InitForOpenGL(window->getSDLWindow(), window->getGLContext());

        ImGui_ImplOpenGL3_Init("#version 450");
        
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        setImGuiStyle();
    }

    void ImGUIModule::shutdown()
    {
        LT_LOGI("ImGuiModule", "Shutdown");
    }

    void ImGUIModule::setImGuiStyle() const
    {
        LT_LOGI("ImGuiModule", "Set imgui style");
        
        ImGuiStyle &style = ImGui::GetStyle();
        ImVec4 *colors = style.Colors;

        // Primary background
        colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);  // #131318
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f); // #131318

        colors[ImGuiCol_PopupBg] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);

        // Headers
        colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.30f, 0.40f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.25f, 0.35f, 1.00f);

        // Buttons
        colors[ImGuiCol_Button] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.32f, 0.40f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.38f, 0.50f, 1.00f);

        // Frame BG
        colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.27f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.35f, 0.35f, 0.50f, 1.00f);
        colors[ImGuiCol_TabActive] = ImVec4(0.25f, 0.25f, 0.38f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.13f, 0.13f, 0.17f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);

        // Title
        colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.20f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);

        // Borders
        colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.25f, 0.50f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

        // Text
        colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.95f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.55f, 1.00f);

        // Highlights
        colors[ImGuiCol_CheckMark] = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.60f, 0.80f, 1.00f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.50f, 0.70f, 1.00f, 0.50f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.60f, 0.80f, 1.00f, 0.75f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.70f, 0.90f, 1.00f, 1.00f);

        // Scrollbar
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.35f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.50f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.45f, 0.45f, 0.55f, 1.00f);

        // Style tweaks
        style.WindowRounding = 5.0f;
        style.FrameRounding = 5.0f;
        style.GrabRounding = 5.0f;
        style.TabRounding = 5.0f;
        style.PopupRounding = 5.0f;
        style.ScrollbarRounding = 5.0f;
        style.WindowPadding = ImVec2(10, 10);
        style.FramePadding = ImVec2(6, 4);
        style.ItemSpacing = ImVec2(8, 6);
        style.PopupBorderSize = 0.f;
    }

    void ImGUIModule::onEvent(const ::SDL_Event& event)
    {
        ImGui_ImplSDL3_ProcessEvent(&event);
    }

    void ImGUIModule::renderUI(float deltaTime) const
    {

        int w = GCM(WindowModule::WindowModule)->getWindow()->getWindowSize().first;
        int h = GCM(WindowModule::WindowModule)->getWindow()->getWindowSize().second;
    
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, w, h);
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.07f, 0.07f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        for (auto& object : m_guiObjects)
        {
            if (object)
            {
                object->render(deltaTime);
            }
        }
        ImGui::ShowMetricsWindow();
        ImGui::Render();
        ImGui::EndFrame();
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    std::weak_ptr<GUIObject> ImGUIModule::addObject(GUIObject* object)
    {
        if (object)
        {
            std::shared_ptr<GUIObject> shared_ptr(object);
            m_guiObjects.push_back(shared_ptr);
            

            LT_LOGI("ImGuiModule", "Add Gui object, id:" + std::to_string(shared_ptr->getID()));
            
            return  {shared_ptr};
        }
        return {};
    }

    void ImGUIModule::removeObject(const std::weak_ptr<GUIObject>& object)
    {
        if (const std::shared_ptr<GUIObject>& shared_ptr = object.lock())
        {
            uint32_t objectID = shared_ptr->getID();
            LT_LOGI("ImGuiModule", "Remove Gui object, id:" + std::to_string(objectID));
            std::erase_if(m_guiObjects, [objectID](const std::shared_ptr<GUIObject>& object)
            {
                return object->getID() == objectID;
            });
        }
    }


}
