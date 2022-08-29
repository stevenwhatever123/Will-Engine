#pragma once
namespace WillEngine::VulkanUtil
{
	std::optional<u32> findQueueFamilies(VkPhysicalDevice& device, VkQueueFlagBits flag, VkSurfaceKHR surface);

	void querySupportedSurfaceFormat(VkPhysicalDevice& device, VkSurfaceKHR& surface,
		VkSurfaceCapabilitiesKHR& capabilities, std::vector<VkSurfaceFormatKHR>& surfaceFormats, std::vector<VkPresentModeKHR>& presentModes);

	bool checkDeviceExtensionSupport(VkPhysicalDevice& device, const std::vector<const char*>& physicalDeviceExtensions);

	VkExtent2D getSwapchainExtent(GLFWwindow* window, VkSurfaceCapabilitiesKHR& capabilities);

	void createImageView(VkDevice& logicalDevice, VkImage& image, VkImageView& imageView, VkFormat format, VkImageAspectFlags aspectMask);
}
