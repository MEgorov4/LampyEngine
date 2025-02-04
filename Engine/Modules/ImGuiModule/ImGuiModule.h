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

/// <summary>
/// Manages the ImGui user interface system, handling GUI object registration and rendering.
/// Implements a singleton pattern to ensure a single instance.
/// </summary>
class ImGuiModule
{
    std::vector<GUIObject*> m_GuiObjects; ///< List of registered GUI objects.

    /// <summary>
    /// Private constructor to enforce singleton pattern.
    /// </summary>
    ImGuiModule() = default;

public:
    /// <summary>
    /// Destructor for the ImGuiModule.
    /// </summary>
    ~ImGuiModule() = default;

    /// <summary>
    /// Retrieves the singleton instance of the ImGuiModule.
    /// </summary>
    /// <returns>Reference to the singleton ImGuiModule instance.</returns>
    static ImGuiModule& getInstance()
    {
        static ImGuiModule ImGuiModule;
        return ImGuiModule;
    }

    /// <summary>
    /// Renders the ImGui user interface.
    /// Calls the render function of all registered GUI objects.
    /// </summary>
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

    /// <summary>
    /// Registers a GUI object to be rendered.
    /// </summary>
    /// <param name="object">Pointer to the GUI object to be added.</param>
    void addObject(GUIObject* object)
    {
        m_GuiObjects.push_back(object);
    }

    /// <summary>
    /// Removes a GUI object from the list using its unique ID.
    /// </summary>
    /// <param name="id">The unique ID of the GUI object to remove.</param>
    void removeObject(uint32_t id)
    {
        std::erase_if(m_GuiObjects, [id](GUIObject* object)
            {
                return object->getID() == id;
            });
    }
};
