#pragma once 
#include <vulkan/vulkan.h>
#include <vector>

class VulkanFramebuffers
{
	std::vector<VkFramebuffer> m_framebuffers;
	VkDevice m_device;

public:
	VulkanFramebuffers(VkDevice device, VkRenderPass renderPass, VkExtent2D extent, std::vector<VkImageView> imageViews);
	~VulkanFramebuffers();
	
	std::vector<VkFramebuffer> getFramebuffers() const { return m_framebuffers; }
private:
	void recreateFramebuffers(VkRenderPass renderPass, VkExtent2D extent, std::vector<VkImageView> imageViews);
	void createFramebuffers(VkRenderPass renderPass, VkExtent2D extent, std::vector<VkImageView> imageViews);
	void clearFramebuffers();
};