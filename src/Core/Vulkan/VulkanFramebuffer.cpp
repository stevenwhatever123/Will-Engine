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

void VulkanFramebuffer::cleanUp(VkDevice& logicalDevice, VmaAllocator& vmaAllocator)
{
	// Clear attachments
	vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);

	vkDestroyImageView(logicalDevice, GBuffer0.imageView, nullptr);
	vmaDestroyImage(vmaAllocator, GBuffer0.vulkanImage.image, GBuffer0.vulkanImage.allocation);

	vkDestroyImageView(logicalDevice, GBuffer1.imageView, nullptr);
	vmaDestroyImage(vmaAllocator, GBuffer1.vulkanImage.image, GBuffer1.vulkanImage.allocation);

	vkDestroyImageView(logicalDevice, GBuffer2.imageView, nullptr);
	vmaDestroyImage(vmaAllocator, GBuffer2.vulkanImage.image, GBuffer2.vulkanImage.allocation);

	vkDestroyImageView(logicalDevice, GBuffer3.imageView, nullptr);
	vmaDestroyImage(vmaAllocator, GBuffer3.vulkanImage.image, GBuffer3.vulkanImage.allocation);

	vkDestroyImageView(logicalDevice, GBuffer4.imageView, nullptr);
	vmaDestroyImage(vmaAllocator, GBuffer4.vulkanImage.image, GBuffer4.vulkanImage.allocation);
}