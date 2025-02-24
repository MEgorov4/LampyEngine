#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <functional>

#include "../IRenderer.h"
#include "VulkanObjects/VulkanGraphicsPipeline.h"

/// <summary>
/// Manages a queue of cleanup functions to be executed during object destruction.
/// </summary>
struct DeletionQueue {
    std::vector<std::function<void()>> deletors; ///< List of deletion functions.

    /// <summary>
    /// Adds a function to the deletion queue.
    /// </summary>
    /// <param name="function">Function to be executed upon flushing.</param>
    void push_function(std::function<void()>&& function) {
        deletors.push_back(function);
    }

    /// <summary>
    /// Executes all stored deletion functions in reverse order and clears the queue.
    /// </summary>
    void flush() {
        for (auto it = deletors.rbegin(); it != deletors.rend(); ++it) {
            (*it)();
        }
        deletors.clear();
    }
};

class Window;
class VulkanInstance;
class VulkanSurface;
class VulkanLogicalDevice;
class VulkanSwapChain;
class VulkanRenderPass;
class VulkanFramebuffers;
class VulkanCommandPool;
class VulkanCommandBuffers;
class VulkanSynchronizationManager;
class VulkanPipelineCache;
class VulkanUniformBuffer;
class VulkanIndexBufferCache;
class VulkanVertexBufferCache;
class VulkanOffscreenRenderer;
class VulkanDescriptorPool;

/// <summary>
/// Handles Vulkan rendering, resource management, and pipeline execution.
/// </summary>
class VulkanRenderer : public IRenderer
{
    std::unique_ptr<VulkanInstance> m_instance;
    std::unique_ptr<VulkanSurface> m_surface;
    std::unique_ptr<VulkanLogicalDevice> m_logicalDevice;
    std::unique_ptr<VulkanSwapChain> m_swapChain;
    std::unique_ptr<VulkanRenderPass> m_renderPass;
    std::unique_ptr<VulkanFramebuffers> m_framebuffers;
    std::unique_ptr<VulkanCommandPool> m_commandPool;
    std::unique_ptr<VulkanCommandBuffers> m_commandBuffers;
    std::unique_ptr<VulkanSynchronizationManager> m_syncManager;
    std::unique_ptr<VulkanPipelineCache> m_pipelineCache;
    std::unique_ptr<VulkanIndexBufferCache> m_indexBufferCache;
    std::unique_ptr<VulkanVertexBufferCache> m_vertexBufferCache;
    std::unique_ptr<VulkanUniformBuffer> m_uniformBuffer;

    std::unique_ptr<VulkanDescriptorPool> m_descriptorPool;
    std::unique_ptr<VulkanOffscreenRenderer> m_offscreenRenderer;
    

    uint32_t m_currentFrame = 0; ///< Tracks the current frame in flight.
    Window* m_window; ///< Pointer to the application window.
    DeletionQueue m_mainDeletionQueue; ///< Queue for deferred cleanup tasks.
    
    bool m_imguiEnabled = false;
public:
    /// <summary>
    /// Constructs the Vulkan renderer and initializes Vulkan.
    /// </summary>
    /// <param name="window">Pointer to the application window.</param>
    explicit VulkanRenderer(Window* window);

    /// <summary>
    /// Destroys the Vulkan renderer and cleans up resources.
    /// </summary>
    ~VulkanRenderer() override;

    /// <summary>
    /// Renders a single frame.
    /// </summary>
    virtual void render() override;

    /// <summary>
    /// Waits for the Vulkan device to become idle.
    /// </summary>
    virtual void waitIdle() override;

    /// <summary>
    /// Registers a shader pipeline.
    /// </summary>
    /// <param name="vertPath">Path to the vertex shader file.</param>
    /// <param name="fragPath">Path to the fragment shader file.</param>
    virtual void registerShader(const std::string& vertPath, const std::string& fragPath) override;

    /// <summary>
    /// Removes a shader pipeline.
    /// </summary>
    /// <param name="vertPath">Path to the vertex shader file.</param>
    /// <param name="fragPath">Path to the fragment shader file.</param>
    virtual void removeShader(const std::string& vertPath, const std::string& fragPath) override;

    /// <summary>
    /// Registers vertex data and creates a Vulkan vertex buffer if needed.
    /// </summary>
    /// <param name="vertexData">Vertex data to register.</param>
    virtual void registerVertexData(const std::vector<Vertex>& vertexData, const std::string& pathToFile) override;

    /// <summary>
    /// Removes vertex data and releases associated buffers if no longer needed.
    /// </summary>
    /// <param name="vertexData">Vertex data to remove.</param>
    virtual void removeVertexData(const std::vector<Vertex>& vertexData, const std::string& pathToFile) override;
    virtual void* getOffscreenImageDescriptor() override;

    /// <summary>
    /// Registers index data and creates a Vulkan index buffer if needed.
    /// </summary>
    /// <param name="indexData">index data to register.</param>
    virtual void registerIndexData(const std::vector<uint32_t>& indexData, const std::string& pathToFile) override;

    /// <summary>
    /// Removes index data and releases associated buffers if no longer needed.
    /// </summary>
    /// <param name="indexData">index data to remove.</param>
    virtual void removeIndexData(const std::vector<uint32_t>& indexData, const std::string& pathToFile) override;

    /// <summary>
    /// Recreates the swap chain and all dependent resources.
    /// </summary>
    void recreateSwapChainAndDependent();

private:
    /// <summary>
    /// Initializes Vulkan and sets up core components.
    /// </summary>
    void initVulkan();

    /// <summary>
    /// Initializes ImGui for Vulkan-based UI rendering.
    /// </summary>
    void initImGui();

    /// <summary>
    /// Executes a command buffer immediately and waits for completion.
    /// </summary>
    /// <param name="function">Lambda function containing Vulkan commands.</param>
    void immediate_submit(std::function<void(VkCommandBuffer)>&& function);

    /// <summary>
    /// Cleans up Vulkan resources before shutting down.
    /// </summary>
    void cleanupVulkan();

    /// <summary>
    /// Draws a single frame.
    /// </summary>
    void drawFrame();

    /// <summary>
    /// Records Vulkan commands for a given frame.
    /// </summary>
    /// <param name="commandBuffer">The command buffer to record into.</param>
    /// <param name="imageIndex">Index of the swap chain image.</param>
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    /// <summary>
    /// Creates rendering commands for the current scene.
    /// </summary>
    /// <param name="commandBuffer">Command buffer to record draw calls into.</param>
    void recordWorldRenderCommands(VkCommandBuffer commandBuffer);

    /// <summary>
    /// Creates the swap chain and all dependent resources.
    /// </summary>
    void createSwapChainAndDependent();

    /// <summary>
    /// Cleans up all resources dependent on the swap chain.
    /// </summary>
    void cleanSwapChainAndDependent();
};
