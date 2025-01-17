#include <gtest/gtest.h>

#include "../Core/Window/Window.h"
#include "../Core/VulkanObjects/VulkanRenderer.h"

TEST(VulkanRendererTest, Construct)
{
	Window* window = new Window(600, 800, "ConstructVulkanRendererTest");


	EXPECT_FALSE(window->shouldClose());

	std::vector<const char*> extensions = window->getRequiredInstanceExtensions();
	EXPECT_FALSE(extensions.size() <= 0);

	VulkanRenderer vulkanRenderer();
}