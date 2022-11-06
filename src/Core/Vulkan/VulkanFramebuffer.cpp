#include "pch.h"
#include "Core/Vulkan/VulkanFramebuffer.h"

VulkanFramebufferAttachment::VulkanFramebufferAttachment():
	vulkanImage({ VK_NULL_HANDLE, VK_NULL_HANDLE }),
	imageView(VK_NULL_HANDLE),
	imguiTextureDescriptorSet(VK_NULL_HANDLE)
{

}

VulkanFramebufferAttachment::~VulkanFramebufferAttachment()
{

}

VulkanFramebuffer::VulkanFramebuffer():
	framebuffer(VK_NULL_HANDLE)
{

}

VulkanFramebuffer::VulkanFramebuffer(VkFramebuffer framebuffer) :
	framebuffer(framebuffer),
	GBuffer1(),
	GBuffer2(),
	GBuffer3(),
	GBuffer4()
{

}

VulkanFramebuffer::~VulkanFramebuffer()
{

}