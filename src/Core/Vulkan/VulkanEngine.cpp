#include "pch.h"
#include "Core/Vulkan/VulkanEngine.h"

VulkanEngine::VulkanEngine():
	vmaAllocator(VK_NULL_HANDLE),
	renderPass(VK_NULL_HANDLE),
	swapchain(VK_NULL_HANDLE),
	swapchainImages(),
	swapchainImageViews(),
	swapchainImageFormat(),
	swapchainExtent(),
	depthImageView(VK_NULL_HANDLE),
	depthImage(VK_NULL_HANDLE),
	depthImageAllocation(VK_NULL_HANDLE),
	framebuffers()
{

}

VulkanEngine::~VulkanEngine()
{

}

void VulkanEngine::init(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR surface)
{
	createVmaAllocator(instance, physicalDevice, logicalDevice);

	createSwapchain(window, logicalDevice, physicalDevice, surface);
	getSwapchainImages(logicalDevice);
	createSwapchainImageViews(logicalDevice);

	createRenderPass(logicalDevice, swapchainImageFormat, depthFormat);

	createDepthBuffer(logicalDevice, vmaAllocator, swapchainExtent);

	createSwapchainFramebuffer(logicalDevice, swapchainImageViews, framebuffers, renderPass, depthImageView, swapchainExtent);

	createCommandPool(logicalDevice, physicalDevice, surface, commandPool);

	createCommandBuffer(logicalDevice, commandBuffers);
}

void VulkanEngine::cleanup(VkDevice& logicalDevice)
{
	// Destroy Command Pool
	vkDestroyCommandPool(logicalDevice, commandPool, nullptr);

	// Destroy framebuffer
	for (auto& framebuffer : framebuffers)
	{
		vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
	}

	// Destroy depth buffer
	vmaDestroyImage(vmaAllocator, depthImage, depthImageAllocation);
	vkDestroyImageView(logicalDevice, depthImageView, nullptr);

	// Destroy swapchain imageview
	for (auto imageView : swapchainImageViews)
	{
		vkDestroyImageView(logicalDevice, imageView, nullptr);
	}

	// Destroy swapchain
	vkDestroySwapchainKHR(logicalDevice, swapchain, nullptr);

	// Destroy Render Pass
	vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
}

bool VulkanEngine::compileShader()
{
	return true;
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

	if (vmaCreateAllocator(&allocatorInfo, &vmaAllocator) != VK_SUCCESS)
		throw std::runtime_error("Failed to create vma allocator");
}

void VulkanEngine::createRenderPass(VkDevice& logicalDevice, VkFormat& format, const VkFormat& depthFormat)
{
	VkAttachmentDescription attachments[2]{};
	// Framebuffer
	attachments[0].format = format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	// Depth buffer
	attachments[1].format = depthFormat;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// The out location of the fragment shader
	// 0 is the color value for the framebuffer
	VkAttachmentReference colorAttachment{};
	colorAttachment.attachment = 0;
	colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// 1 is the depth value for the depth buffer
	VkAttachmentReference depthAttachment{};
	depthAttachment.attachment = 1;
	depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpasses[1]{};
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = 1;
	subpasses[0].pColorAttachments = &colorAttachment;
	subpasses[0].pDepthStencilAttachment = &depthAttachment;

	VkRenderPassCreateInfo passInfo{};
	passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	passInfo.attachmentCount = static_cast<u32>(sizeof(attachments) / sizeof(attachments[0]));
	passInfo.pAttachments = attachments;
	passInfo.subpassCount = static_cast<u32>(sizeof(subpasses) / sizeof(subpasses[0]));
	passInfo.pSubpasses = subpasses;

	if (vkCreateRenderPass(logicalDevice, &passInfo, nullptr, &renderPass) != VK_SUCCESS)
		throw std::runtime_error("Failed to create render pass");
}

void VulkanEngine::createSwapchain(GLFWwindow* window, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface)
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> availableSurfaceFormats;
	std::vector<VkPresentModeKHR> presentModes;

	WillEngine::VulkanUtil::querySupportedSurfaceFormat(physicalDevice, surface, capabilities, availableSurfaceFormats, presentModes);

	VkExtent2D extent = WillEngine::VulkanUtil::getSwapchainExtent(window, capabilities);
	// Store extent for later use
	swapchainExtent = extent;
	VkSurfaceFormatKHR surfaceFormat = selectSwapchainSurfaceFormat(availableSurfaceFormats);
	// Store image format for later use
	swapchainImageFormat = surfaceFormat.format;
	VkPresentModeKHR presentMode = selectSwapchainPresentMode(presentModes);

	// Make sure we have enough image for swapchain and make sure it does not exceed the maximum number
	if (numSwapchainImage < capabilities.minImageCount)
		throw std::runtime_error("Number of swapchain desired is less than the minimum required");
	if (numSwapchainImage > capabilities.maxImageCount)
		throw std::runtime_error("Number of swapchain desired is more than the maximum supported");

	VkSwapchainCreateInfoKHR swapchainInfo{};
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.surface = surface;
	swapchainInfo.minImageCount = numSwapchainImage;
	swapchainInfo.imageFormat = surfaceFormat.format;
	swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainInfo.imageExtent = extent;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	std::optional<u32> graphicsFamilyIndicies = WillEngine::VulkanUtil::findQueueFamilies(physicalDevice, VK_QUEUE_GRAPHICS_BIT, VK_NULL_HANDLE);
	std::optional<u32> presentFamilyIndicies = WillEngine::VulkanUtil::findQueueFamilies(physicalDevice, VK_QUEUE_GRAPHICS_BIT, surface);

	if (graphicsFamilyIndicies != presentFamilyIndicies)
	{
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainInfo.queueFamilyIndexCount = 2;
		u32 queueFamilyIndicies[] = { graphicsFamilyIndicies.value(), presentFamilyIndicies.value() };
		swapchainInfo.pQueueFamilyIndices = queueFamilyIndicies;
	}
	else
	{
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainInfo.queueFamilyIndexCount = 1;
	}

	swapchainInfo.preTransform = capabilities.currentTransform;
	swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainInfo.presentMode = presentMode;
	swapchainInfo.clipped = VK_TRUE;


	if (vkCreateSwapchainKHR(logicalDevice, &swapchainInfo, nullptr, &swapchain) != VK_SUCCESS)
		throw std::runtime_error("Failed to create swapchain");
}

void VulkanEngine::getSwapchainImages(VkDevice& logicalDevice)
{
	u32 imageCount = 0;
	vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, nullptr);

	swapchainImages.resize(imageCount);

	vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, swapchainImages.data());
}

void VulkanEngine::createSwapchainImageViews(VkDevice& logicalDevice)
{
	swapchainImageViews.resize(swapchainImages.size());

	for (u32 i = 0; i < swapchainImageViews.size(); i++)
	{
		WillEngine::VulkanUtil::createImageView(logicalDevice, swapchainImages[i], swapchainImageViews[i], swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

void VulkanEngine::createDepthBuffer(VkDevice& logicalDevice, VmaAllocator& vmaAllocator, const VkExtent2D& swapchainExtent)
{
	VkExtent3D depthExtent = { swapchainExtent.width, swapchainExtent.height, 1 };

	VkImageCreateInfo depthImageInfo{};
	depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
	depthImageInfo.format = depthFormat;
	depthImageInfo.extent = depthExtent;
	depthImageInfo.mipLevels = 1;
	depthImageInfo.arrayLayers = 1;
	depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	depthImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VmaAllocationCreateInfo depthImageAllocationInfo{};
	depthImageAllocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	// Create depth buffer
	if (vmaCreateImage(vmaAllocator, &depthImageInfo, &depthImageAllocationInfo, &depthImage, &depthImageAllocation, nullptr) != VK_SUCCESS)
		throw std::runtime_error("Failed to create depth image");

	// Create depth buffer image view
	WillEngine::VulkanUtil::createImageView(logicalDevice, depthImage, depthImageView, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VulkanEngine::createSwapchainFramebuffer(VkDevice& logicalDevice, std::vector<VkImageView>& swapchainImageViews,
	std::vector<VkFramebuffer>& framebuffers, VkRenderPass& renderPass, VkImageView& depthImageView, VkExtent2D swapchainExtent)
{
	assert(framebuffers.empty());

	for (u32 i = 0; i < swapchainImageViews.size(); i++)
	{
		VkImageView attachments[2]{};
		attachments[0] = swapchainImageViews[i];
		attachments[1] = depthImageView;

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<u32>(sizeof(attachments) / sizeof(attachments[0]));
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapchainExtent.width;
		framebufferInfo.height = swapchainExtent.height;
		framebufferInfo.layers = 1;

		VkFramebuffer framebuffer = VK_NULL_HANDLE;

		if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS)
			throw std::runtime_error("Failed to create framebuffer");

		framebuffers.emplace_back(std::move(framebuffer));
	}

	assert(swapchainImageViews.size() == framebuffers.size());
}

void VulkanEngine::createCommandPool(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkCommandPool& commandPool)
{
	std::optional<u32> graphicsFamilyIndex = WillEngine::VulkanUtil::findQueueFamilies(physicalDevice, VK_QUEUE_GRAPHICS_BIT, surface);

	if (!graphicsFamilyIndex.has_value())
		throw std::runtime_error("Cannot retrieve graphics family index");

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = graphicsFamilyIndex.value();

	if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create command pool");
}

void VulkanEngine::createCommandBuffer(VkDevice& logicalDevice, std::vector<VkCommandBuffer>& commandBuffers)
{
	commandBuffers.resize(framebuffers.size());

	for (u32 i = 0; i < commandBuffers.size(); i++)
	{
		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = commandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(logicalDevice, &allocateInfo, &commandBuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate command buffers");
	}
}

// =========================================================================================================
// Private helper functions

VkSurfaceFormatKHR VulkanEngine::selectSwapchainSurfaceFormat(std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats)
{
	for (const auto& availableFormat : availableSurfaceFormats)
	{
		if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	// Return the first element if none of them satisfie the requirement    
	return availableSurfaceFormats[0];
}

VkPresentModeKHR VulkanEngine::selectSwapchainPresentMode(std::vector<VkPresentModeKHR>& presentModes)
{
	for (const auto& availableMode : presentModes)
	{
		if (availableMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availableMode;
		}
	}

	// Use FIFO if MAILBOX is not availabe
	return VK_PRESENT_MODE_FIFO_KHR;
}