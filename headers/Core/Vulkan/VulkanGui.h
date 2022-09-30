#pragma once

class VulkanGui
{
private:

	VkDescriptorPool imguiDescriptorPool = VK_NULL_HANDLE;

	VkCommandBuffer imguiCommandBuffer = VK_NULL_HANDLE;

public:

	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

public:

	VulkanGui();
	~VulkanGui();

	void init(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface,
		u32 queueFamily, VkCommandPool& commandPool, VkDescriptorPool& descriptorPool, u32 imageCount, VkRenderPass& renderPass, VkExtent2D extent);

	void cleanUp(VkDevice& logicalDevice);

	void update();

	void renderUI(VkCommandBuffer& commandBuffer, VkExtent2D extent);
};