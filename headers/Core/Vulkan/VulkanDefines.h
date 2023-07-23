#pragma once

struct VulkanAllocatedMemory
{
	VkBuffer buffer;
	VmaAllocation allocation;
};

struct VulkanAllocatedImage
{
	VkImage image;
	VkImageView imageView;
	VmaAllocation allocation;
};

struct VulkanDescriptorSet
{
	VkDescriptorSet descriptorSet;
	VkDescriptorSetLayout layout;
};

struct VulkanPipeline
{
	VkPipeline pipeline;
	VkPipelineLayout layout;
};

class VulkanFramebufferAttachment
{
public:

	VulkanAllocatedImage vulkanImage;
	VkImageView imageView;
	VkFormat format;

	// For ImGui UI
	VkDescriptorSet imguiTextureDescriptorSet;

public:

	VulkanFramebufferAttachment();
	~VulkanFramebufferAttachment();
};

class VulkanFramebuffer
{
public:

	static const u32 attachmentSize = 4;

public:

	VkFramebuffer framebuffer;
	VulkanFramebufferAttachment GBuffer0;
	VulkanFramebufferAttachment GBuffer1;
	VulkanFramebufferAttachment GBuffer2;
	VulkanFramebufferAttachment GBuffer3;

public:

	VulkanFramebuffer();
	VulkanFramebuffer(VkFramebuffer framebuffer);
	~VulkanFramebuffer();

	void cleanUp(VkDevice& logicalDevice, VmaAllocator& vmaAllocator);
};