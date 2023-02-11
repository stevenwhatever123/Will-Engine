#pragma once
#include "Core/Vulkan/VulkanAllocatedObject.h"
#include "Core/Vulkan/VulkanFramebuffer.h"

#include "Core/Material.h"

#include "Managers/FileManager.h"

#include "Utils/Image.h"

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

	// Viewport
	VkViewport getViewport(VkExtent2D extent);

	// Scissor
	VkRect2D getScissor(VkExtent2D extent);

	//Images
	VulkanAllocatedImage createImage(VkDevice& logicalDevice, VmaAllocator& vmaAllocator, VkFormat format, VkImageUsageFlags usage, u32 width, 
		u32 height, u32 mipLevels);

	VulkanAllocatedImage createImageWithFlags(VkDevice& logicalDevice, VmaAllocator& vmaAllocator, VkFormat format, VkImageUsageFlags usage, VkImageCreateFlags flags,
		u32 width, u32 height, u32 mipLevels, u32 arrayLayers);

	VulkanAllocatedImage createImageWithLayouts(VkDevice& logicalDevice, VmaAllocator& vmaAllocator, VkFormat format, VkImageUsageFlags usage, VkImageLayout layout,
		u32 width, u32 height, u32 mipLevels);

	void loadTextureImage(VkDevice& logicalDevice, VmaAllocator vmaAllocator, VkCommandPool& commandPool, VkQueue& queue, VulkanAllocatedImage& vulkanImage, 
		u32 mipLevels, u32 width, u32 height, unsigned char* textureImage);

	void loadTextureImageWithMipmap(VkDevice& logicalDevice, VmaAllocator vmaAllocator, VkCommandPool& commandPool, VkQueue& queue, VulkanAllocatedImage& vulkanImage,
		u32 mipLevels, u32 width, u32 height, unsigned char* textureImage);

	u32 calculateMiplevels(u32 width, u32 height);

	void createImageView(VkDevice& logicalDevice, VkImage& image, VkImageView& imageView, u32 mipLevels, VkFormat format, 
		VkImageAspectFlags aspectMask);

	void createDepthImageView(VkDevice& logicalDevice, VkImage& image, VkImageView& imageView, u32 mipLevels, VkFormat format,
		VkImageAspectFlags aspectMask);

	void setImageLayout(VkCommandBuffer& commandBuffer, VkImage& image, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout,
		VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	// Samplers
	void createDefaultSampler(VkDevice& logicalDevice, VkSampler& sampler);

	void createTextureSampler(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSampler& sampler, u32 mipLevels);

	void createAttachmentSampler(VkDevice& logicalDevice, VkSampler& sampler);

	void createDepthSampler(VkDevice& logicalDevice, VkSampler& sampler);

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
	void initSkeletalShaderModule(VkDevice& logicalDevice, VkShaderModule& vertShader, VkShaderModule& fragShader);
	void initGeometryShaderModule(VkDevice& logicalDevice, VkShaderModule& vertShader, VkShaderModule& fragShader);
	void initShadingShaderModule(VkDevice& logicalDevice, VkShaderModule& vertShader, VkShaderModule& fragShader);
	void initShadowShaderModule(VkDevice& logicalDevice, VkShaderModule& vertShader, VkShaderModule& geomShader, VkShaderModule& fragShader);
	void initDepthShaderModule(VkDevice& logicalDevice, VkShaderModule& vertShader, VkShaderModule& fragShader);
	void initDepthSkeletonShaderModule(VkDevice& logicalDevice, VkShaderModule& vertShader, VkShaderModule& fragShader);

	void initFilterBrightShaderModule(VkDevice& logicalDevice, VkShaderModule& compShader);
	void initClearColorShaderModule(VkDevice& logicalDevice, VkShaderModule& compShader);
	void initDownscaleShaderModule(VkDevice& logicalDevice, VkShaderModule& compShader);
	void initUpscaleShaderModule(VkDevice& logicalDevice, VkShaderModule& compShader);
	void initBlendColorShaderModule(VkDevice& logicalDevice, VkShaderModule& compShader);

	// Descriptors related
	void createDescriptorSetLayout(VkDevice& logicalDevice, VkDescriptorSetLayout& descriptorSetLayout,VkDescriptorType descriptorType, 
		VkShaderStageFlags shaderStage, u32 binding, u32 descriptorCount);

	void allocDescriptorSet(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool, VkDescriptorSetLayout& descriptorSetInfo,
		VkDescriptorSet& descriptorSet);

	void writeDescriptorSetBuffer(VkDevice& logicalDevice, VkDescriptorSet& descriptorSet, VkBuffer& descriptorBuffer, u32 binding);

	void writeDescriptorSetImage(VkDevice& logicalDevice, VkDescriptorSet& descriptorSet, VkSampler* sampler,
		VkImageView* imageView, VkImageLayout imageLayout, VkDescriptorType descriptorType, u32 binding, u32 descriptorCount);

	// Pipeline
	void createPipelineLayout(VkDevice& logicalDevice, VkPipelineLayout& pipelineLayout, u32 size,
		VkDescriptorSetLayout* descriptorSetLayout, u32 pushConstantCount, VkPushConstantRange* pushConstant);

	void createPipeline(VkDevice& logicalDevice, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, VkRenderPass& renderpass, 
		VkShaderModule& vertShader, VkShaderModule& fragShader, VkPrimitiveTopology primitive, VkExtent2D extent);
	void createSkeletalPipeline(VkDevice& logicalDevice, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, VkRenderPass& renderpass,
		VkShaderModule& vertShader, VkShaderModule& fragShader, VkPrimitiveTopology primitive, VkExtent2D extent);
	void createGeometryPipeline(VkDevice& logicalDevice, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, VkRenderPass& renderpass,
		VkShaderModule& vertShader, VkShaderModule& fragShader, VkPrimitiveTopology primitive, VkExtent2D extent);
	void createShadingPipeline(VkDevice& logicalDevice, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, VkRenderPass& renderpass,
		VkShaderModule& vertShader, VkShaderModule& fragShader, VkPrimitiveTopology primitive, VkExtent2D extent);
	void createShadowPipeline(VkDevice& logicalDevice, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, VkRenderPass& renderpass,
		VkShaderModule& vertShader, VkShaderModule& geomShader, VkShaderModule& fragShader, VkPrimitiveTopology primitive, u32 width, u32 height);
	void createDepthPipeline(VkDevice& logicalDevice, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, VkRenderPass& renderpass,
		VkShaderModule& vertShader, VkShaderModule& fragShader, VkPrimitiveTopology primitive, VkExtent2D extent);
	void createDepthSkeletalPipeline(VkDevice& logicalDevice, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, VkRenderPass& renderpass,
		VkShaderModule& vertShader, VkShaderModule& fragShader, VkPrimitiveTopology primitive, VkExtent2D extent);

	void createComputePipeline(VkDevice& logicalDevice, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, VkShaderModule& compShader);

	// Framebuffer
	void createFramebufferAttachment(VkDevice& logicalDevice, VmaAllocator& vmaAllocator, VkFormat format, VkExtent2D extent, 
		VulkanFramebufferAttachment& attachment);
	void createShadingImage(VkDevice& logicalDevice, VmaAllocator& vmaAllocator, VkFormat format, VkExtent2D extent, VulkanAllocatedImage& image);
	void createComputedImage(VkDevice& logicalDevice, VmaAllocator& vmaAllocator, VkCommandPool& commandPool, VkQueue& graphicsQueue, VkFormat format, 
		VkExtent2D extent, VulkanAllocatedImage& image);
}