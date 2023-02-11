#pragma once
#include "Core/Vulkan/VulkanAllocatedObject.h"

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