#include "pch.h"
#include "Core/Vulkan/VulkanDefines.h"

// VulkanFramebufferAttachment
VulkanFramebufferAttachment::VulkanFramebufferAttachment() :
	vulkanImage({ VK_NULL_HANDLE, VK_NULL_HANDLE }),
	imageView(VK_NULL_HANDLE),
	imguiTextureDescriptorSet(VK_NULL_HANDLE)
{

}

VulkanFramebufferAttachment::~VulkanFramebufferAttachment()
{

}

// VulkanFramebuffer
VulkanFramebuffer::VulkanFramebuffer() :
	framebuffer(VK_NULL_HANDLE)
{

}

VulkanFramebuffer::VulkanFramebuffer(VkFramebuffer framebuffer) :
	framebuffer(framebuffer),
	attachments()
{

}

VulkanFramebuffer::~VulkanFramebuffer()
{

}

void VulkanFramebuffer::cleanUp(VkDevice& logicalDevice, VmaAllocator& vmaAllocator)
{
	// Clear attachments
	vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);

	for (VulkanFramebufferAttachment& attachment : attachments)
	{
		vkDestroyImageView(logicalDevice, attachment.imageView, nullptr);
		vmaDestroyImage(vmaAllocator, attachment.vulkanImage.image, attachment.vulkanImage.allocation);
	}
}