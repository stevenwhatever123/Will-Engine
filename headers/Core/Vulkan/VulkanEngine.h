#pragma once

#include "Utils/VulkanUtil.h"

class VulkanEngine
{
private:

public:

	// Triple buffering
	const u32 numSwapchainImage = 3;

	const VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

public:

	VmaAllocator vmaAllocator;

	VkRenderPass renderPass;

	// Swapchain
	VkSwapchainKHR swapchain;

	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;

	VkFormat swapchainImageFormat;

	VkExtent2D swapchainExtent;

	// Depth buffer
	VkImageView depthImageView;
	VkImage depthImage;
	VmaAllocation depthImageAllocation;

	// Framebuffer
	// Stored as a vector as there are multiple framebuffers in a swapchain
	std::vector<VkFramebuffer> framebuffers;

public:

	VulkanEngine();
	~VulkanEngine();

	void init(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR surface);
	void cleanup(VkDevice& logicalDevice);

	void createVmaAllocator(VkInstance& instance, VkPhysicalDevice& physicalDevice, VkDevice& logicalDevice);

	void createSwapchain(GLFWwindow* window, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);
	void getSwapchainImages(VkDevice& logicalDevice);
	void createSwapchainImageViews(VkDevice& logicalDevice);

	void createRenderPass(VkDevice& logicalDevice, VkFormat& format, const VkFormat& depthFormat);

	void createDepthBuffer(VkDevice& logicalDevice, VmaAllocator& vmaAllocator, const VkExtent2D& swapchainExtent);

	void createSwapchainFramebuffer(VkDevice& logicalDevice, std::vector<VkImageView>& swapchainImageViews,
		std::vector<VkFramebuffer>& framebuffers, VkRenderPass& renderPass, VkImageView& depthImageView, VkExtent2D swapchainExtent);

private:
	VkSurfaceFormatKHR selectSwapchainSurfaceFormat(std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats);

	VkPresentModeKHR selectSwapchainPresentMode(std::vector<VkPresentModeKHR>& presentModes);
};