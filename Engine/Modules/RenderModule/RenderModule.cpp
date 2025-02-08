#include "RenderModule.h"

#include <vulkan/vulkan.h>

VkDescriptorSet RenderModule::getVulkanOffscreenImageView()
{
	return m_renderer->getVulkanOffscreenImageView();
}
