#pragma once

class VulkanEngine
{
private:

public:

public:

	VmaAllocator vmaAllocator;

	VkRenderPass renderPass;

public:

	VulkanEngine();
	~VulkanEngine();

	void init(VkInstance& instance, VkPhysicalDevice& physicalDevice, VkDevice& logicalDevice, VkFormat& format);
	void cleanup(VkDevice& logicalDevice);

	void createVmaAllocator(VkInstance& instance, VkPhysicalDevice& physicalDevice, VkDevice& logicalDevice);

	void createRenderPass(VkDevice& logicalDevice, VkFormat& format);

	void createPipelineLayout();

};