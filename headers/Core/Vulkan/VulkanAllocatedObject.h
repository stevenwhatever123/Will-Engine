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