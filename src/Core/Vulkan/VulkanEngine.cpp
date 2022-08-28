#include "pch.h"
#include "Core/Vulkan/VulkanEngine.h"

VulkanEngine::VulkanEngine():
	vmaAllocator(VK_NULL_HANDLE),
	renderPass(VK_NULL_HANDLE)
{

}

VulkanEngine::~VulkanEngine()
{

}

void VulkanEngine::init(VkInstance& instance, VkPhysicalDevice& physicalDevice, VkDevice& logicalDevice, VkFormat& format)
{
	createVmaAllocator(instance, physicalDevice, logicalDevice);
	createRenderPass(logicalDevice, format);
}

void VulkanEngine::cleanup(VkDevice& logicalDevice)
{
	vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
}


void VulkanEngine::createVmaAllocator(VkInstance& instance, VkPhysicalDevice& physicalDevice, VkDevice& logicalDevice)
{
	VmaVulkanFunctions functions{};
	functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
	functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = physicalDevice;
	allocatorInfo.device = logicalDevice;
	allocatorInfo.pVulkanFunctions = &functions;
	allocatorInfo.instance = instance;
	//allocatorInfo.vulkanApiVersion = properties.apiVersion;
	allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;

	VmaAllocator allocator;

	if (vmaCreateAllocator(&allocatorInfo, &allocator) != VK_SUCCESS)
		throw std::runtime_error("Failed to create vma allocator");
}

void VulkanEngine::createRenderPass(VkDevice& logicalDevice, VkFormat& format)
{
	VkAttachmentDescription attachments[1]{};
	attachments[0].format = format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// The out location of the fragment shader
	// 0 is the color
	VkAttachmentReference subpassAttachments[1]{};
	subpassAttachments[0].attachment = 0;
	subpassAttachments[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpasses[1]{};
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = 1;
	subpasses[0].pColorAttachments = subpassAttachments;

	VkRenderPassCreateInfo passInfo{};
	passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	passInfo.attachmentCount = static_cast<u32>(sizeof(attachments) / sizeof(attachments[0]));
	passInfo.pAttachments = attachments;
	passInfo.subpassCount = static_cast<u32>(sizeof(subpasses) / sizeof(subpasses[0]));
	passInfo.pSubpasses = subpasses;

	if (vkCreateRenderPass(logicalDevice, &passInfo, nullptr, &renderPass) != VK_SUCCESS)
		throw std::runtime_error("Failed to create render pass");
}

void VulkanEngine::createPipelineLayout()
{
	
}