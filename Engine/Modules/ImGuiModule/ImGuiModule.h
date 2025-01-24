#pragma once 
#include "../../ThirdParty/ImGui_backends/imgui_impl_vulkan.h"
#include "../../ThirdParty/ImGui_backends/imgui_impl_glfw.h"
#include "../WindowModule/WindowModule.h"
class ImGuiModule
{
public:
    ImGuiModule() {}
    ~ImGuiModule() {}
    static ImGuiModule& getInstance()
    {
        static ImGuiModule ImGuiModule;
        return ImGuiModule;
    }

    void startUp()
    {

    }

    void renderUI()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Hello");
        ImGui::End();
        ImGui::Render();
        ImGui::EndFrame();
    }

    void shutDown()
    {
    }
};