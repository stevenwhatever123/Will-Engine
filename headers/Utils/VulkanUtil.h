#pragma once
namespace WillEngine::VulkanUtil
{
	std::optional<u32> findQueueFamilies(VkPhysicalDevice& device, VkQueueFlagBits flag, VkSurfaceKHR surface);

	void querySupportedSurfaceFormat(VkPhysicalDevice& device, VkSurfaceKHR& surface,
		VkSurfaceCapabilitiesKHR& capabilities, std::vector<VkSurfaceFormatKHR>& surfaceFormats, std::vector<VkPresentModeKHR>& presentModes);

	bool checkDeviceExtensionSupport(VkPhysicalDevice& device, const std::vector<const char*>& physicalDeviceExtensions);

	VkExtent2D getSwapchainExtent(GLFWwindow* window, VkSurfaceCapabilitiesKHR& capabilities);

	void createImageView(VkDevice& logicalDevice, VkImage& image, VkImageView& imageView, VkFormat format, VkImageAspectFlags aspectMask);

	std::tuple<VkBuffer, VmaAllocation> createBuffer(VmaAllocator& vmaAllocator, u64 allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

	VkCommandPool createCommandPool(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);

	VkCommandBuffer createCommandBuffer(VkDevice& logicalDevice, VkCommandPool& commandPool);

	VkFence createFence(VkDevice& logicalDevice, bool signaled);

	void bufferBarrier(VkCommandBuffer& commandBuffer, VkBuffer& buffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStage,
		VkPipelineStageFlags dstStage, VkDeviceSize size, VkDeviceSize offset, u32 srcFamilyIndex, u32 dstFamilyIndex);

	VkShaderModule createShaderModule(VkDevice& logicalDevice, std::vector<char>& shaderCode);
}