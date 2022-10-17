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

	VkFramebuffer framebuffer;
	VulkanFramebufferAttachment position;
	VulkanFramebufferAttachment normal;
	VulkanFramebufferAttachment emissive;
	VulkanFramebufferAttachment ambient;
	VulkanFramebufferAttachment albedo;
	VulkanFramebufferAttachment metallic;
	VulkanFramebufferAttachment roughness;

public:

	VulkanFramebuffer();
	VulkanFramebuffer(VkFramebuffer framebuffer);
	~VulkanFramebuffer();
};