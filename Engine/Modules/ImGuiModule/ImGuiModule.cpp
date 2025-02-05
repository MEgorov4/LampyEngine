#include "ImGuiModule.h"

#include <algorithm>
#include <imgui.h>

#include "GLFWBackends/imgui_impl_glfw.h"
#include "VulkanBackends/imgui_impl_vulkan.h"

#include "GUIObject.h"

#include "../LoggerModule/Logger.h"

void ImGuiModule::renderUI() const
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    for (auto& object : m_GuiObjects)
    {
        assert(object);
        object->render();
    }
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
