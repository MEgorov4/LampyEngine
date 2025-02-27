#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <array>
#include <stdexcept>
#include <functional>

#include "../VulkanObjects/VulkanUniformBuffer.h"
#include "../VulkanObjects/VulkanIndexBufferCache.h"
#include "../VulkanObjects/VulkanVertexBufferCache.h"

struct Vertex;
class VulkanPipelineCache;

/// <summary>
/// Класс, реализующий оффскрин-рендеринг для генерации текстуры, которую можно показать в ImGui (например, в viewport).
/// Для работы требуется передать в конструктор уже созданные объекты: логическое устройство, физическое устройство,
/// command pool, графическую очередь и размеры оффскрин-фреймбуфера.
/// </summary>
class VulkanOffscreenRenderer 
{
public:
    /// <summary>
    /// Конструктор. Параметры:
    /// - device: логическое устройство Vulkan
    /// - physicalDevice: физическое устройство Vulkan
    /// - extent: размеры оффскрин-рендеринга (ширина и высота)
    /// - commandPool: пул команд, из которого будут выделяться командные буферы для операций (например, для переходов)
    /// - graphicsQueue: графическая очередь (для немедленной отправки команд)
    /// </summary>
    VulkanOffscreenRenderer(VkDevice inDevice,
        VkPhysicalDevice inPhysicalDevice,
        VkExtent2D inExtent,
        VkCommandPool inCommandPool,
        VkQueue inGraphicsQueue,
        VkDescriptorPool inDescriptorPool,
        VkSurfaceKHR inSurface,
        const std::vector<VkImageView>& inImageViews,
        VkFormat inSwapChainImageFormat)
        : device(inDevice),
        physicalDevice(inPhysicalDevice),
        extent(inExtent),
        commandPool(inCommandPool),
        graphicsQueue(inGraphicsQueue),
        descriptorPool(inDescriptorPool),
        surface(inSurface),
        m_imageViews(inImageViews),
        swapChainImageFormat(inSwapChainImageFormat)
    {
        init();
    }

    ~VulkanOffscreenRenderer();

    /// <summary>
    /// Начинает оффскрин-рендер пасс, устанавливая область отрисовки, viewport и scissor.
    /// Вызывается внутри командного буфера.
    /// </summary>
    void beginRenderPass(VkCommandBuffer cmd, uint32_t imageIndex);

    /// <summary>
    /// Завершает оффскрин-рендер пасс внутри командного буфера.
    /// </summary>
    void endRenderPass(VkCommandBuffer cmd);

    /// <summary>
    /// Возвращает VkImageView цветового вложения.
    /// </summary>
    const std::vector<VkDescriptorSet>& getColorImageDescriptors() const { return descriptorSets; }
    VkDescriptorSet getColorImageDescriptor(uint32_t currentFrame) const { return descriptorSets[currentFrame]; }

    /// <summary>
    /// Возвращает VkSampler для цветового вложения (при необходимости для дескриптора в шейдере).
    /// </summary>
    VkSampler getColorSampler() const { return colorSampler; }


    VkRenderPass getOffscreenRenderPass() const { return renderPass; }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, VulkanPipelineCache* vulkanCache, uint32_t currentFrame);
    void recordWorldRenderCommands(VkCommandBuffer commandBuffer, VulkanPipelineCache* vulkanCache, uint32_t currentFrame);
    void submitOffscreenRender(VkCommandBuffer commandBuffer, VkQueue graphicsQueue, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore);

    void registerVertexData(const std::vector<Vertex>& vertexData, const std::string& pathToFile);
    void removeVertexData(const std::vector<Vertex>& vertexData, const std::string& pathToFile);
    void registerIndexData(const std::vector<uint32_t>& indexData, const std::string& pathToFile);
    void removeIndexData(const std::vector<uint32_t>& indexData, const std::string& pathToFile);

    void createTextureImage();
    void createTextureImageView();

    void updateUniform(uint32_t currentImage);
private:
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkExtent2D extent;
    VkCommandPool commandPool;
    VkQueue graphicsQueue;
    VkSurfaceKHR surface;
    std::vector<VkImageView> m_imageViews;
    VkFormat swapChainImageFormat;
    
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    // Ресурсы для цветового вложения (в которое рендерится изображение)
    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;
    VkSampler colorSampler;

    // Ресурсы для глубинного вложения
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    VkImage textureImage;
    VkImageView textureImageView;
    VkDeviceMemory textureImageMemory;

    // Render pass и framebuffer для оффскрин-рендеринга
    VkRenderPass renderPass;
    std::vector<VkFramebuffer> framebuffers;
    
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    std::vector<VkDescriptorSet> descriptorSets;

    std::unique_ptr<VulkanIndexBufferCache> m_indexBufferCache;
    std::unique_ptr<VulkanVertexBufferCache> m_vertexBufferCache;
    std::unique_ptr<VulkanUniformBuffer> m_uniformBuffer;

    uint32_t mipLevels;

    // Инициализация всех объектов оффскрин-рендеринга
    void init();

    // Создаёт изображение и представление для цветового вложения.
    void createColorResources();

    // Создаёт изображение и представление для глубинного вложения.
    void createDepthResources();

    // Создаёт render pass с двумя вложениями: цветовым и глубинным.
    void createRenderPass();

    void createDescriptorSetLayout();
    void allocateOffscreenDescriptorSet();
    void updateOffscreenDescriptorSet();
    // Создаёт framebuffer, связывающий созданные представления с render pass.
    void createFramebuffer();

    // Создаёт сэмплер для цветового вложения (при дальнейшей выборке в шейдере).
    void createSampler();

    // Универсальная функция для создания VkImage с указанными параметрами.
    void createImage(VkFormat format, VkImageUsageFlags usage, VkImage& image, VkDeviceMemory& memory);

    // Создаёт представление (VkImageView) для заданного изображения.
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    // Находит формат для глубинного вложения.
    VkFormat findDepthFormat() {
        return findSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    // Перебирает кандидаты и возвращает первый поддерживаемый формат.
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
        VkImageTiling tiling,
        VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
        throw std::runtime_error("failed to find supported format!");
    }

    // Находит индекс подходящего типа памяти.
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && ((memProperties.memoryTypes[i].propertyFlags & properties) == properties)) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }
};
