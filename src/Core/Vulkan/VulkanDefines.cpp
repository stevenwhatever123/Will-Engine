#include "pch.h"
#include "Core/Vulkan/VulkanDefines.h"

// VulkanFramebuffer
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