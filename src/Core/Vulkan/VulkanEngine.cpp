#include "pch.h"

#include "Core/Vulkan/VulkanEngine.h"

VulkanEngine::VulkanEngine() :
	camera(nullptr),
	meshes(),
	materials(),
	lights(),
	vmaAllocator(VK_NULL_HANDLE),
	renderPass(VK_NULL_HANDLE),
	swapchain(VK_NULL_HANDLE),
	swapchainImages(),
	swapchainImageViews(),
	swapchainImageFormat(),
	swapchainExtent(),
	depthImageView(VK_NULL_HANDLE),
	depthImage({ VK_NULL_HANDLE, VK_NULL_HANDLE }),
	framebuffers(),
	commandPool(VK_NULL_HANDLE),
	commandBuffers(),
	imageAvailable(VK_NULL_HANDLE),
	renderFinished(VK_NULL_HANDLE),
	fences(),
	descriptorPool(VK_NULL_HANDLE),
	defaultPipelineLayout(VK_NULL_HANDLE),
	defaultPipeline(VK_NULL_HANDLE),
	brdfMetallicPipelineLayout(VK_NULL_HANDLE),
	brdfMetallicPipeline(VK_NULL_HANDLE),
	sceneDescriptorSetLayout(VK_NULL_HANDLE),
	sceneDescriptorSet(VK_NULL_HANDLE),
	sceneUniformBuffer({ VK_NULL_HANDLE, VK_NULL_HANDLE }),
	lightDescriptorSetLayout(VK_NULL_HANDLE),
	lightDescriptorSet(VK_NULL_HANDLE),
	lightUniformBuffer({ VK_NULL_HANDLE, VK_NULL_HANDLE }),
	cameraDescriptorSetLayout(VK_NULL_HANDLE),
	cameraDescriptorSet(VK_NULL_HANDLE),
	cameraUniformBuffer({ VK_NULL_HANDLE, VK_NULL_HANDLE }),
	textureDescriptorSetLayout(VK_NULL_HANDLE),
	defaultVertShader(VK_NULL_HANDLE),
	defaultFragShader(VK_NULL_HANDLE),
	vulkanGui(nullptr),
	sceneMatrix(1)
{

}

VulkanEngine::~VulkanEngine()
{

}

void VulkanEngine::init(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR surface, VkQueue& queue)
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

	createSemaphore(logicalDevice, imageAvailable, renderFinished);

	createFence(logicalDevice, fences, VK_FENCE_CREATE_SIGNALED_BIT);

	createDescriptionPool(logicalDevice);

	// Scene Descriptors for scene matrix with binding 0 in vertex shader
	initUniformBuffer(logicalDevice, descriptorPool, sceneUniformBuffer, sceneDescriptorSet, sceneDescriptorSetLayout, 0, sizeof(mat4), VK_SHADER_STAGE_VERTEX_BIT);

	// Light Descriptors for light with binding 0 in fragment shader
	initUniformBuffer(logicalDevice, descriptorPool, lightUniformBuffer, lightDescriptorSet, lightDescriptorSetLayout, 0, 
		sizeof(LightUniform), VK_SHADER_STAGE_FRAGMENT_BIT);

	// Camera Descriptors for camera position with binding 0 in fragment shader
	initUniformBuffer(logicalDevice, descriptorPool, cameraUniformBuffer, cameraDescriptorSet, cameraDescriptorSetLayout, 0, sizeof(vec4), VK_SHADER_STAGE_FRAGMENT_BIT);

	// Texture Descriptor with binding 1 in fragment shader
	// We only need to know the layout of the descriptor
	// Phong
	//WillEngine::VulkanUtil::createDescriptorSetLayout(logicalDevice, textureDescriptorSetLayout, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	//	VK_SHADER_STAGE_FRAGMENT_BIT, 1, 4);
	// BRDF
	WillEngine::VulkanUtil::createDescriptorSetLayout(logicalDevice, textureDescriptorSetLayout, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT, 1, 4);

	// Shader Modules
	WillEngine::VulkanUtil::initShaderModule(logicalDevice, defaultVertShader, defaultFragShader);

	// Create pipeline and pipeline layout
	VkDescriptorSetLayout layouts[] = { sceneDescriptorSetLayout, lightDescriptorSetLayout, cameraDescriptorSetLayout, textureDescriptorSetLayout };
	u32 descriptorSetLayoutSize = sizeof(layouts) / sizeof(layouts[0]);

	VkPushConstantRange pushConstants[1];
	// Push constant object for model matrix to be used in vertex shader
	pushConstants[0].offset = 0;
	pushConstants[0].size = sizeof(mat4);
	pushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	// Push constant object for uniform material to be used in fragment shader
	//pushConstants[1].offset = sizeof(mat4);
	//pushConstants[1].size = sizeof(vec4) * 4;
	//pushConstants[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	u32 pushConstantCount = sizeof(pushConstants) / sizeof(pushConstants[0]);

	// Create pipeline layout with our just created push constant
	WillEngine::VulkanUtil::createPipelineLayout(logicalDevice, defaultPipelineLayout,
		descriptorSetLayoutSize, layouts, pushConstantCount, pushConstants);

	// Create pipeline
	WillEngine::VulkanUtil::createPipeline(logicalDevice, defaultPipeline, defaultPipelineLayout,
		renderPass, defaultVertShader, defaultFragShader, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, swapchainExtent);

	// Gui
	initGui(window, instance, logicalDevice, physicalDevice, queue, surface);
}

void VulkanEngine::cleanup(VkDevice& logicalDevice)
{
	// Wait for all execution to be finished before cleaning up
	vkDeviceWaitIdle(logicalDevice);

	// Destroy Gui
	vulkanGui->cleanUp(logicalDevice);
	delete vulkanGui;

	// Destroy Descriptor sets / layouts
	// Scene
	vmaDestroyBuffer(vmaAllocator, sceneUniformBuffer.buffer, sceneUniformBuffer.allocation);
	vkFreeDescriptorSets(logicalDevice, descriptorPool, 1, &sceneDescriptorSet);
	vkDestroyDescriptorSetLayout(logicalDevice, sceneDescriptorSetLayout, nullptr);
	// Light
	vmaDestroyBuffer(vmaAllocator, lightUniformBuffer.buffer, lightUniformBuffer.allocation);
	vkFreeDescriptorSets(logicalDevice, descriptorPool, 1, &lightDescriptorSet);
	vkDestroyDescriptorSetLayout(logicalDevice, lightDescriptorSetLayout, nullptr);
	// Camera
	vmaDestroyBuffer(vmaAllocator, cameraUniformBuffer.buffer, cameraUniformBuffer.allocation);
	vkFreeDescriptorSets(logicalDevice, descriptorPool, 1, &cameraDescriptorSet);
	vkDestroyDescriptorSetLayout(logicalDevice, cameraDescriptorSetLayout, nullptr);
	// Texture
	vkDestroyDescriptorSetLayout(logicalDevice, textureDescriptorSetLayout, nullptr);

	// Destroy default pipeline and pipeline layout
	vkDestroyPipeline(logicalDevice, defaultPipeline, nullptr);
	vkDestroyPipelineLayout(logicalDevice, defaultPipelineLayout, nullptr);

	// Destroy default shader modules
	vkDestroyShaderModule(logicalDevice, defaultVertShader, nullptr);
	vkDestroyShaderModule(logicalDevice, defaultFragShader, nullptr);

	// Destroy all data from a material
	for (auto* material : materials)
	{
		material->cleanUp(logicalDevice, vmaAllocator, descriptorPool);
		delete material;
	}

	// Destroy all data from a mesh
	for (auto* mesh : meshes)
	{
		mesh->cleanup(logicalDevice, vmaAllocator);
		delete mesh;
	}

	// Destroy Descriptor Pool
	vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);

	// Destroy Fences
	for (auto& fence : fences)
	{
		vkDestroyFence(logicalDevice, fence, nullptr);
	}

	// Destroy Semaphore
	vkDestroySemaphore(logicalDevice, imageAvailable, nullptr);
	vkDestroySemaphore(logicalDevice, renderFinished, nullptr);

	// Free Command Buffer and Destroy Command Pool
	vkFreeCommandBuffers(logicalDevice, commandPool, commandBuffers.size(), commandBuffers.data());
	vkDestroyCommandPool(logicalDevice, commandPool, nullptr);

	// Destroy framebuffer
	for (auto& framebuffer : framebuffers)
	{
		vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
	}

	// Destroy depth buffer
	vmaDestroyImage(vmaAllocator, depthImage.image, depthImage.allocation);
	vkDestroyImageView(logicalDevice, depthImageView, nullptr);

	// Destroy swapchain imageview
	for (auto imageView : swapchainImageViews)
	{
		vkDestroyImageView(logicalDevice, imageView, nullptr);
	}

	// Destroy vma alloator
	vmaDestroyAllocator(vmaAllocator);

	// Destroy swapchain
	vkDestroySwapchainKHR(logicalDevice, swapchain, nullptr);

	// Destroy Render Pass
	vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
}

void VulkanEngine::update(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR surface, 
	VkQueue graphicsQueue)
{
	// Acquire next image of the swapchain
	u32 imageIndex = 0;
	const VkResult res = vkAcquireNextImageKHR(logicalDevice, swapchain, std::numeric_limits<u64>::max(), imageAvailable, VK_NULL_HANDLE, &imageIndex);

	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
	{
		printf("HAHA you resized the window!\n");
		
		vkDeviceWaitIdle(logicalDevice);

		recreateSwapchain(window, logicalDevice, physicalDevice, surface);

		return;
	}

	if (res != VK_SUCCESS)
		throw std::runtime_error("Failed to acquire swapchain image index");

	// Wait for command buffer to be available
	if (vkWaitForFences(logicalDevice, 1, &fences[imageIndex], VK_TRUE, std::numeric_limits<u64>::max()) != VK_SUCCESS)
		throw std::runtime_error("Failed to wait for fences to be available");

	if (vkResetFences(logicalDevice, 1, &fences[imageIndex]) != VK_SUCCESS)
		throw std::runtime_error("Failed to reset fence");

	assert(imageIndex < commandBuffers.size());
	assert(imageIndex < fences.size());

	// Make sure command buffer finishes executing
	vkResetCommandBuffer(commandBuffers[imageIndex], 0);

	// Update Texture
	if (updateTexture || updateColor)
	{
		changeMaterialTexture(logicalDevice, physicalDevice, graphicsQueue, updateTexture, updateColor, selectedMaterialIndex, selectedTextureIndex, textureFilepath);
		textureFilepath = "";
	}

	// Update ImGui UI
	updateGui(logicalDevice, graphicsQueue);

	recordCommands(commandBuffers[imageIndex], renderPass, framebuffers[imageIndex], swapchainExtent);

	submitCommands(commandBuffers[imageIndex], imageAvailable, renderFinished, graphicsQueue, fences[imageIndex]);

	presentImage(graphicsQueue, renderFinished, swapchain, imageIndex);
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
		WillEngine::VulkanUtil::createImageView(logicalDevice, swapchainImages[i], swapchainImageViews[i], 1, swapchainImageFormat, 
			VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

void VulkanEngine::recreateSwapchain(GLFWwindow* window, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface)
{
	auto const oldFormat = swapchainImageFormat;
	auto const oldExtent = swapchainExtent;

	// Destroy old objects
	VkSwapchainKHR oldSwapchain = swapchain;

	for (auto view : swapchainImageViews)
		vkDestroyImageView(logicalDevice, view, nullptr);

	swapchainImageViews.clear();
	swapchainImages.clear();

	// Destroy the old swapchain
	vkDestroySwapchainKHR(logicalDevice, oldSwapchain, nullptr);

	// Create new swapchains
	createSwapchain(window, logicalDevice, physicalDevice, surface);

	// Get swap chain image and create associate image views
	getSwapchainImages(logicalDevice);
	createSwapchainImageViews(logicalDevice);

	// Determine which chain properties have changed and change render pass or depth buffer accordingly
	bool extentChanged = false;
	bool formatChanged = false;
	if (oldExtent.width != swapchainExtent.width || oldExtent.height != swapchainExtent.height)
		extentChanged = true;

	if (oldFormat != swapchainImageFormat)
		formatChanged = true;

	if (formatChanged)
		createRenderPass(logicalDevice, swapchainImageFormat, depthFormat);

	if (extentChanged)
	{
		// Destroy the old depth buffer
		vkDestroyImageView(logicalDevice, depthImageView, nullptr);
		vmaDestroyImage(vmaAllocator, depthImage.image, depthImage.allocation);

		// Create a new depth buffer
		createDepthBuffer(logicalDevice, vmaAllocator, swapchainExtent);
	}

	// Recreate framebuffers
	for (auto framebuffer : framebuffers)
		vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);

	framebuffers.clear();

	createSwapchainFramebuffer(logicalDevice, swapchainImageViews, framebuffers, renderPass, depthImageView, swapchainExtent);

	if (extentChanged)
	{
		// Destroy old pipeline
		vkDestroyPipeline(logicalDevice, defaultPipeline, nullptr);

		// Create new pipeline
		WillEngine::VulkanUtil::createPipeline(logicalDevice, defaultPipeline, defaultPipelineLayout, renderPass, defaultVertShader,
			defaultFragShader, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, swapchainExtent);
	}

	//vkDeviceWaitIdle(logicalDevice);
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
	if (vmaCreateImage(vmaAllocator, &depthImageInfo, &depthImageAllocationInfo, &depthImage.image, &depthImage.allocation, nullptr) != VK_SUCCESS)
		throw std::runtime_error("Failed to create depth image");

	// Create depth buffer image view
	WillEngine::VulkanUtil::createImageView(logicalDevice, depthImage.image, depthImageView, 1, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
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
	commandPool = WillEngine::VulkanUtil::createCommandPool(logicalDevice, physicalDevice, surface);
}

void VulkanEngine::createCommandBuffer(VkDevice& logicalDevice, std::vector<VkCommandBuffer>& commandBuffers)
{
	commandBuffers.resize(framebuffers.size());

	for (u32 i = 0; i < commandBuffers.size(); i++)
	{
		commandBuffers[i] = WillEngine::VulkanUtil::createCommandBuffer(logicalDevice, commandPool);
	}
}

void VulkanEngine::createSemaphore(VkDevice& logicalDevice, VkSemaphore& waitImageAvailable, VkSemaphore& signalRenderFinish)
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &waitImageAvailable) != VK_SUCCESS)
		throw std::runtime_error("Failed to create wait image availabe semaphore");

	if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &signalRenderFinish) != VK_SUCCESS)
		throw std::runtime_error("Failed to create signal render finish semaphore");
}

void VulkanEngine::createFence(VkDevice& logicalDevice, std::vector<VkFence>& fences, VkFenceCreateFlagBits flag)
{
	fences.resize(framebuffers.size());

	for (u32 i = 0; i < fences.size(); i++)
	{
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = flag;

		if (vkCreateFence(logicalDevice, &fenceInfo, nullptr, &fences[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create fence");
	}
}

void VulkanEngine::createDescriptionPool(VkDevice& logicalDevice)
{
	VkDescriptorPoolSize const pools[] = {
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2048},
			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2048}
	};

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.maxSets = 1024;
	poolInfo.poolSizeCount = sizeof(pools) / sizeof(pools[0]);
	poolInfo.pPoolSizes = pools;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool);
}

void VulkanEngine::initUniformBuffer(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool, VulkanAllocatedMemory& uniformBuffer, 
	VkDescriptorSet& descriptorSet, VkDescriptorSetLayout& descriptorSetLayout, u32 binding, u32 bufferSize, VkShaderStageFlagBits shaderStage)
{
	createUniformBuffer(logicalDevice, uniformBuffer, bufferSize);

	WillEngine::VulkanUtil::createDescriptorSetLayout(logicalDevice, descriptorSetLayout, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		shaderStage, binding, 1);

	WillEngine::VulkanUtil::allocDescriptorSet(logicalDevice, descriptorPool, descriptorSetLayout, descriptorSet);

	WillEngine::VulkanUtil::writeDescriptorSetBuffer(logicalDevice, descriptorSet, uniformBuffer.buffer, binding);
}

void VulkanEngine::createUniformBuffer(VkDevice& logicalDevice, VulkanAllocatedMemory& uniformBuffer, u32 bufferSize)
{
	uniformBuffer =
		WillEngine::VulkanUtil::createBuffer(vmaAllocator, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
}

void VulkanEngine::initGui(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkQueue& queue,
	VkSurfaceKHR& surface)
{
	vulkanGui = new VulkanGui();

	std::optional<u32> graphicsFamilyIndicies = WillEngine::VulkanUtil::findQueueFamilies(physicalDevice, VK_QUEUE_GRAPHICS_BIT, VK_NULL_HANDLE);

	if (!graphicsFamilyIndicies.has_value())
		std::runtime_error("Failed to get graphics queue family index");

	vulkanGui->init(window, instance, logicalDevice, physicalDevice, surface, graphicsFamilyIndicies.value(), commandPool, descriptorPool, numSwapchainImage, renderPass,
		swapchainExtent);
}

void VulkanEngine::updateGui(VkDevice& logicalDevice, VkQueue& graphicsQueue)
{
	vulkanGui->update(meshes, materials, lights, updateTexture, updateColor, selectedMaterialIndex, selectedTextureIndex, textureFilepath);
}

void VulkanEngine::updateSceneUniform(Camera* camera)
{
	// Temperory!!!!!
	// Model Transform
	mat4 model(1);

	// Update camera 
	sceneMatrix = camera->getProjectionMatrix(swapchainExtent.width, swapchainExtent.height) * camera->getCameraMatrix() * model;
}

void VulkanEngine::updateLightUniform(Camera* camera)
{
	// Update Light's Position
	for (auto light : lights)
	{
		light->lightUniform.transformedPosition = vec4(light->position.x, light->position.y, light->position.z, 1);
	}
}

void VulkanEngine::recordCommands(VkCommandBuffer& commandBuffer, VkRenderPass& renderPass, VkFramebuffer& framebuffer, VkExtent2D& extent)
{
	// Begin command buffer
	VkCommandBufferBeginInfo commandBeginInfo{};
	commandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(commandBuffer, &commandBeginInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to begin command buffer");

	// Update uniform buffers
	vkCmdUpdateBuffer(commandBuffer, sceneUniformBuffer.buffer, 0, sizeof(mat4), &sceneMatrix);

	// Update light uniform buffers
	vkCmdUpdateBuffer(commandBuffer, lightUniformBuffer.buffer, 0, sizeof(lights[0]->lightUniform), &lights[0]->lightUniform);

	vec4 cameraPosition = vec4(camera->position, 1);

	// Update camera uniform buffers
	vkCmdUpdateBuffer(commandBuffer, cameraUniformBuffer.buffer, 0, sizeof(vec4), &cameraPosition);

	VkClearValue clearValue[2];
	// Clear color
	clearValue[0].color.float32[0] = 0.5f;
	clearValue[0].color.float32[1] = 0.5f;
	clearValue[0].color.float32[2] = 0.5f;
	clearValue[0].color.float32[3] = 1.0f;
	// Clear Depth
	clearValue[1].depthStencil.depth = 1.0f;

	// Begin Render Pass
	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = framebuffer;
	renderPassBeginInfo.renderArea.extent = extent;
	renderPassBeginInfo.clearValueCount = static_cast<u32>(sizeof(clearValue) / sizeof(clearValue[0]));
	renderPassBeginInfo.pClearValues = clearValue;

	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	// ========================================================
	// Recording all the rendering command

	// Rendering pass
	renderPasses(commandBuffer);

	// UI rendering pass
	UIPasses(commandBuffer, extent);

	// ========================================================

	// End Render Pass
	vkCmdEndRenderPass(commandBuffer);

	// End command buffer
	vkEndCommandBuffer(commandBuffer);
}

void VulkanEngine::renderPasses(VkCommandBuffer& commandBuffer)
{
	// Bind default pipeline
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPipeline);

	for (auto mesh : meshes)
	{
		VkBuffer buffers[3] = { mesh->positionBuffer.buffer, mesh->normalBuffer.buffer, mesh->uvBuffer.buffer };

		VkDeviceSize offsets[3]{};

		// Bind buffers
		vkCmdBindVertexBuffers(commandBuffer, 0, 3, buffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer, mesh->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		// Bind Scene Uniform Buffer
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPipelineLayout, 0, 1, &sceneDescriptorSet, 0, nullptr);

		// Bind Light Uniform Buffer
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPipelineLayout, 1, 1, &lightDescriptorSet, 0, nullptr);

		// Bind Camera Uniform Buffer
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPipelineLayout, 2, 1, &cameraDescriptorSet, 0, nullptr);

		// Bind Texture
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPipelineLayout, 3, 1, &materials[mesh->materialIndex]->textureDescriptorSet, 0, nullptr);

		// Push constant for model matrix
		mesh->updateModelMatrix();
		vkCmdPushConstants(commandBuffer, defaultPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mesh->modelMatrix), &mesh->modelMatrix);

		// Push constant for material uniform
		//vkCmdPushConstants(commandBuffer, defaultPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(mat4),
		//	sizeof(materials[mesh->materialIndex]->phongMaterialUniform), &materials[mesh->materialIndex]->phongMaterialUniform);

		vkCmdDrawIndexed(commandBuffer, static_cast<u32>(mesh->indiciesSize), 3, 0, 0, 0);
	}
}

void VulkanEngine::UIPasses(VkCommandBuffer& commandBuffer, VkExtent2D extent)
{
	// ImGui UI rendering
	vulkanGui->renderUI(commandBuffer, extent);
}

void VulkanEngine::submitCommands(VkCommandBuffer& commandBuffer, VkSemaphore& waitSemaphore, VkSemaphore& signalSemaphore,
	VkQueue& graphicsQueue, VkFence& fence)
{
	VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &waitSemaphore;
	submitInfo.pWaitDstStageMask = &dstStageMask;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &signalSemaphore;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence);
}

void VulkanEngine::presentImage(VkQueue& graphicsQueue, VkSemaphore& waitSemaphore, VkSwapchainKHR& swapchain, u32& swapchainIndex)
{
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &waitSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &swapchainIndex;

	vkQueuePresentKHR(graphicsQueue, &presentInfo);
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

void VulkanEngine::changeMaterialTexture(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue, bool& updateTexture, 
	bool& updateColor, u32 materialIndex, u32 textureIndex, std::string filename)
{
	vkDeviceWaitIdle(logicalDevice);

	if (updateTexture)
	{
		// Load the texture first, if we cannot read the texture, we proceed to changing the color
		Image* textureImage = new Image();
		materials[materialIndex]->textures[textureIndex].textureImage = textureImage;

		// Don't update the texture path if it's the same one or an empty string
		if (filename.compare("") != 0 &&
			(materials[materialIndex]->textures[textureIndex].texture_path.size() < 1 || materials[materialIndex]->textures[textureIndex].texture_path.compare(filename) != 0))
		{
			materials[materialIndex]->textures[textureIndex].texture_path = filename;
		}

		materials[materialIndex]->textures[textureIndex].textureImage->readImage(materials[materialIndex]->textures[textureIndex].texture_path.c_str(),
			materials[materialIndex]->textures[textureIndex].width, materials[materialIndex]->textures[textureIndex].height, materials[materialIndex]->textures[textureIndex].numChannels);

		// If we cannot load the texture, create a color texture
		if (materials[materialIndex]->textures[textureIndex].textureImage->data == NULL)
		{
			delete textureImage;
			materials[materialIndex]->textures[textureIndex].texture_path = "";
			updateColor = true;
		}
		
		updateTexture = false;
	}

	if (updateColor)
	{
		// Create new data
		Image* textureImage = new Image();

		materials[materialIndex]->textures[textureIndex].textureImage = textureImage;

		materials[materialIndex]->textures[textureIndex].width = 1;
		materials[materialIndex]->textures[textureIndex].height = 1;

		switch (textureIndex)
		{
		case 0:
			materials[materialIndex]->textures[textureIndex].textureImage->setImageColor(materials[materialIndex]->phongMaterialUniform.emissiveColor);
			break;
		case 1:
			materials[materialIndex]->textures[textureIndex].textureImage->setImageColor(materials[materialIndex]->phongMaterialUniform.ambientColor);
			break;
		case 2:
			materials[materialIndex]->textures[textureIndex].textureImage->setImageColor(materials[materialIndex]->phongMaterialUniform.diffuseColor);
			break;
		case 3:
			materials[materialIndex]->textures[textureIndex].textureImage->setImageColor(materials[materialIndex]->phongMaterialUniform.specularColor);
			break;
		}

		materials[materialIndex]->textures[textureIndex].has_texture = false;

		updateColor = false;
	}

	// Update the associated descriptor set
	materials[materialIndex]->updateDescriptorSet(logicalDevice, physicalDevice, vmaAllocator, commandPool, descriptorPool, graphicsQueue, textureIndex);
}