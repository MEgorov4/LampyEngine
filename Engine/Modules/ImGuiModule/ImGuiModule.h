#pragma once 

#include <memory>
#include <vector>
#include <algorithm>
#include <format>

#include <imgui.h>

#include "GLFWBackends/imgui_impl_glfw.h"
#include "VulkanBackends/imgui_impl_vulkan.h"

#include "GUIObject.h"
#include "../LoggerModule/Logger.h"

class ImGuiModule
{
    std::vector<GUIObject*> m_GuiObjects;

    ImGuiModule() = default;
public:
    ~ImGuiModule() = default;
    static ImGuiModule& getInstance()
    {
        static ImGuiModule ImGuiModule;
        return ImGuiModule;
    }

    void renderUI()
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
    
    void addObject(GUIObject* object)
    {
        m_GuiObjects.push_back(object);
    }

    void removeObject(uint32_t id)
    {
        std::erase_if(m_GuiObjects, [id](GUIObject* object)
        {
            return object->getID() == id;
        });
    }
};