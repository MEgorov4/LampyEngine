#include "ImGuiModule.h"

#include <algorithm>
#include <imgui.h>

#include "GLFWBackends/imgui_impl_glfw.h"
#include "VulkanBackends/imgui_impl_vulkan.h"

#include "GUIObject.h"

#include "../LoggerModule/Logger.h"

void ImGuiModule::startup()
{
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

void ImGuiModule::shutDown()
{
}

void ImGuiModule::renderUI() const
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
    for (auto& object : m_GuiObjects)
    {
        assert(object);
        object->render();
    }
    //Optional logic for update multi-viewport system
    //if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    //{
    //    ImGui::UpdatePlatformWindows();
    //    ImGui::RenderPlatformWindowsDefault();
    //}
    ImGui::Render();
    ImGui::EndFrame();
}

void ImGuiModule::addObject(GUIObject* object)
{
    m_GuiObjects.push_back(object);
}

void ImGuiModule::removeObject(uint32_t id)
{
    std::erase_if(m_GuiObjects, [id](GUIObject* object)
        {return object->getID() == id; });
}
