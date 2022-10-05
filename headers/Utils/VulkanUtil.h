#pragma once
#include "Managers/FileManager.h"

#include "Utils/Image.h"

struct VulkanAllocatedMemory
{
	VkBuffer buffer;
	VmaAllocation allocation;
};

struct VulkanAllocatedImage
{
	VkImage image;
	VmaAllocation allocation;
};

namespace WillEngine::VulkanUtil
{
	// Queue Families
	std::optional<u32> findQueueFamilies(VkPhysicalDevice& device, VkQueueFlagBits flag, VkSurfaceKHR surface);

	// Surfaces
	void querySupportedSurfaceFormat(VkPhysicalDevice& device, VkSurfaceKHR& surface,
		VkSurfaceCapabilitiesKHR& capabilities, std::vector<VkSurfaceFormatKHR>& surfaceFormats, std::vector<VkPresentModeKHR>& presentModes);

	// Extension Support
	bool checkDeviceExtensionSupport(VkPhysicalDevice& device, const std::vector<const char*>& physicalDeviceExtensions);

	// Swapchain Details
	VkExtent2D getSwapchainExtent(GLFWwindow* window, VkSurfaceCapabilitiesKHR& capabilities);

	//Images
	VulkanAllocatedImage createImage(VkDevice& logicalDevice, VmaAllocator& vmaAllocator, VkImage& image, VkFormat format, u32 width, u32 height, u32 mipLevel);

	void loadTextureImage(VkDevice& logicalDevice, VmaAllocator vmaAllocator, VkCommandPool& commandPool, VkQueue& queue, VulkanAllocatedImage& vulkanImage, 
		u32 mipLevels, u32 width, u32 height, unsigned char* textureImage);

	void loadTextureImageWithMipmap(VkDevice& logicalDevice, VmaAllocator vmaAllocator, VkCommandPool& commandPool, VkQueue& queue, VulkanAllocatedImage& vulkanImage,
		u32 mipLevels, u32 width, u32 height, unsigned char* textureImage);

	u32 calculateMiplevels(u32 width, u32 height);

	void createImageView(VkDevice& logicalDevice, VkImage& image, VkImageView& imageView, u32 mipLevels, VkFormat format, 
		VkImageAspectFlags aspectMask);

	// Samplers
	void createDefaultSampler(VkDevice& logicalDevice, VkSampler& sampler);

	void createTextureSampler(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSampler& sampler, u32 mipLevels);

	// Buffers
	VulkanAllocatedMemory createBuffer(VmaAllocator& vmaAllocator, u64 allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

	// Command Pool / Buffer
	VkCommandPool createCommandPool(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);

	VkCommandBuffer createCommandBuffer(VkDevice& logicalDevice, VkCommandPool& commandPool);

	// Fances
	VkFence createFence(VkDevice& logicalDevice, bool signaled);

	// Memory Buffer Barrier
	void bufferBarrier(VkCommandBuffer& commandBuffer, VkBuffer& buffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStage,
		VkPipelineStageFlags dstStage, VkDeviceSize size, VkDeviceSize offset, u32 srcFamilyIndex, u32 dstFamilyIndex);

	void imageBarrier(VkCommandBuffer& commandBuffer, VkImage& image, VkAccessFlags srcAccessFlag, VkAccessFlags dstAccessFlag,
		VkImageLayout srcLayout, VkImageLayout dstLayout, VkImageSubresourceRange subresourceRange, u32 srcQueueFamilyIndex, u32 dstQueueFamilyIndex,
		VkPipelineStageFlags srcStageFlag, VkPipelineStageFlags dstStageFlag);

	// ShaderModule
	VkShaderModule createShaderModule(VkDevice& logicalDevice, std::vector<char>& shaderCode);
	void initShaderModule(VkDevice& logicalDevice, VkShaderModule& vertShader, VkShaderModule& fragShader);

	// Descriptors related
	void createDescriptorSetLayout(VkDevice& logicalDevice, VkDescriptorSetLayout& descriptorSetLayout,VkDescriptorType descriptorType, VkShaderStageFlags shaderStage);

	void allocDescriptorSet(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool, VkDescriptorSetLayout& descriptorSetInfo,
		VkDescriptorSet& descriptorSet);

	// Pipeline
	void createPipelineLayout(VkDevice& logicalDevice, VkPipelineLayout& pipelineLayout, u32 size,
		VkDescriptorSetLayout* descriptorSetLayout);

	void createPipeline(VkDevice& logicalDevice, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, VkRenderPass& renderpass, 
		VkShaderModule& vertShader, VkShaderModule& fragShader, VkPrimitiveTopology primitive, VkExtent2D swapchainExtent);
}