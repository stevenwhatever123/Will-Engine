#pragma once

class VulkanGui
{
private:

public:

	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

public:

	VulkanGui();
	~VulkanGui();

	void init(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, u32 queueFamily,
		VkDescriptorPool& descriptorPool, u32 imageCount, VkRenderPass& renderPass);

	void update();
};