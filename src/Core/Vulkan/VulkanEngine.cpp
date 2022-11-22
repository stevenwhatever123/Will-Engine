#include "pch.h"

#include "Core/Vulkan/VulkanEngine.h"

VulkanEngine::VulkanEngine() :
	camera(nullptr),
	vmaAllocator(VK_NULL_HANDLE),
	geometryRenderPass(VK_NULL_HANDLE),
	shadingRenderPass(VK_NULL_HANDLE),
	swapchain(VK_NULL_HANDLE),
	swapchainImageFormat(),
	depthImageView(VK_NULL_HANDLE),
	depthImage({ VK_NULL_HANDLE, VK_NULL_HANDLE }),
	framebuffers(),
	offscreenFramebuffer(),
	attachmentSampler(VK_NULL_HANDLE),
	commandPool(VK_NULL_HANDLE),
	commandBuffers(),
	imageAvailable(VK_NULL_HANDLE),
	renderFinished(VK_NULL_HANDLE),
	fences(),
	descriptorPool(VK_NULL_HANDLE),
	geometryPipelineLayout(VK_NULL_HANDLE),
	geometryPipeline(VK_NULL_HANDLE),
	shadingPipelineLayout(VK_NULL_HANDLE),
	shadingPipeline(VK_NULL_HANDLE),
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
	geometryVertShader(VK_NULL_HANDLE),
	geometryFragShader(VK_NULL_HANDLE),
	shadingVertShader(VK_NULL_HANDLE),
	shadingFragShader(VK_NULL_HANDLE),
	vulkanGui(nullptr),
	sceneMatrix()
{

}

VulkanEngine::~VulkanEngine()
{

}

void VulkanEngine::init(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR surface, VkQueue& graphicsQueue,
	GameState* gameState)
{
	this->gameState = gameState;

	// Create / Allocate resources
	createVmaAllocator(instance, physicalDevice, logicalDevice);
	createCommandPool(logicalDevice, physicalDevice, surface, commandPool);
	createCommandBuffer(logicalDevice, commandBuffers, computeCommandBuffers, presentCommandBuffers);
	createSemaphore(logicalDevice, imageAvailable, renderFinished, readyToPresent, computeFinished);
	createFence(logicalDevice, fences, VK_FENCE_CREATE_SIGNALED_BIT);
	createDescriptionPool(logicalDevice);

	// Create swapchain
	createSwapchain(window, logicalDevice, physicalDevice, surface);
	getSwapchainImages(logicalDevice);
	createSwapchainImageViews(logicalDevice);

	// Create Render Pass
	createDepthPrePass(logicalDevice, depthRenderPass, depthFormat);
	createShadowRenderPass(logicalDevice, shadowRenderPass, shadowDepthFormat);
	createGeometryRenderPass(logicalDevice, geometryRenderPass, VK_FORMAT_R16G16B16A16_SFLOAT, depthFormat);
	createShadingRenderPass(logicalDevice, shadingRenderPass, swapchainImageFormat, depthFormat);
	createPresentRenderPass(logicalDevice, presentRenderPass, swapchainImageFormat);

	// Create and allocate GBuffer resources
	createDepthBuffer(logicalDevice, vmaAllocator, sceneExtent);
	createOffscreenAttachments(logicalDevice, sceneExtent);

	WillEngine::VulkanUtil::createShadingImage(logicalDevice, vmaAllocator, swapchainImageFormat, sceneExtent, shadingImage, shadingImageView);
	createShadingFramebuffer(logicalDevice, shadingFramebuffer, shadingRenderPass, sceneExtent);
	WillEngine::VulkanUtil::createAttachmentSampler(logicalDevice, attachmentSampler);

	WillEngine::VulkanUtil::createComputedImage(logicalDevice, vmaAllocator, commandPool, graphicsQueue, swapchainImageFormat, sceneExtent, postProcessedImage, postProcessedImageView);

	// Create and allocate framebuffer for presenting
	createSwapchainFramebuffer(logicalDevice, swapchainImageViews, framebuffers, offscreenFramebuffer, geometryRenderPass, shadingRenderPass, depthImageView, swapchainExtent);

	// Gui
	initGui(window, instance, logicalDevice, physicalDevice, graphicsQueue, surface);

	// Used in mostly all passes
	// Scene Descriptors for scene matrix with binding 0 in vertex shader
	initUniformBuffer(logicalDevice, descriptorPool, sceneUniformBuffer, sceneDescriptorSet, sceneDescriptorSetLayout, 0, sizeof(CameraMatrix), VK_SHADER_STAGE_VERTEX_BIT);

	// For shading phase
	// Light Descriptors for light with binding 1 in fragment shader
	initUniformBuffer(logicalDevice, descriptorPool, lightUniformBuffer, lightDescriptorSet, lightDescriptorSetLayout, 1, sizeof(LightUniform), VK_SHADER_STAGE_FRAGMENT_BIT);

	// For Shadowing mapping
	// Light Descriptor for light view projection with binding 2 in geometry shader
	initUniformBuffer(logicalDevice, descriptorPool, lightMatrixUniformBuffer, lightMatrixDescriptorSet, lightMatrixDescriptorSetLayout, 2, sizeof(mat4) * 6,
		VK_SHADER_STAGE_GEOMETRY_BIT);

	// Camera View Projection
	// Camera Descriptors for camera position with binding 1 in fragment shader
	initUniformBuffer(logicalDevice, descriptorPool, cameraUniformBuffer, cameraDescriptorSet, cameraDescriptorSetLayout, 1, sizeof(vec4), VK_SHADER_STAGE_FRAGMENT_BIT);

	// Phong: 4, BRDF: 5
	u32 descriptorSize = 5;

	// Texture Descriptor with binding 1 in fragment shader
	// We only need to know the layout of the descriptor
	WillEngine::VulkanUtil::createDescriptorSetLayout(logicalDevice, textureDescriptorSetLayout, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT, 1, descriptorSize);

	WillEngine::VulkanUtil::createDefaultSampler(logicalDevice, defaultSampler);


	// Graphics Pipeline
	initDepthPipeline(logicalDevice);
	initShadowPipeline(logicalDevice);
	initGeometryPipeline(logicalDevice);
	initShadingPipeline(logicalDevice);

	// Descriptor Set for the final shaded image to be used in the UI rendering
	initRenderedDescriptors(logicalDevice, descriptorPool);

	initComputedImageDescriptors(logicalDevice, descriptorPool);

	// Compute Pipeline for bloom
	initBloomDownscalePipeline(logicalDevice);
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

	// Destroy pipeline and pipeline layout
	vkDestroyPipeline(logicalDevice, geometryPipeline, nullptr);
	vkDestroyPipelineLayout(logicalDevice, geometryPipelineLayout, nullptr);

	// Destroy default shader modules
	vkDestroyShaderModule(logicalDevice, geometryVertShader, nullptr);
	vkDestroyShaderModule(logicalDevice, geometryFragShader, nullptr);

	// Destroy all data from a material
	for (auto* material : gameState->graphicsResources.materials)
	{
		material->cleanUp(logicalDevice, vmaAllocator, descriptorPool);
		delete material;
	}

	// Destroy all data from a mesh
	for (auto* mesh : gameState->graphicsResources.meshes)
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
	vkDestroyRenderPass(logicalDevice, shadingRenderPass, nullptr);
}

void VulkanEngine::update(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR surface, 
	VkQueue graphicsQueue, bool renderWithBRDF)
{
	// Acquire next image of the swapchain
	u32 imageIndex = 0;
	const VkResult res = vkAcquireNextImageKHR(logicalDevice, swapchain, std::numeric_limits<u64>::max(), imageAvailable, VK_NULL_HANDLE, &imageIndex);

	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || sceneExtentChanged)
	{	
		vkDeviceWaitIdle(logicalDevice);

		recreateSwapchainFramebuffer(window, logicalDevice, physicalDevice, surface, graphicsQueue);

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
	vkResetCommandBuffer(computeCommandBuffers[imageIndex], 0);
	vkResetCommandBuffer(presentCommandBuffers[imageIndex], 0);

	// Rendering command

	recordCommands(commandBuffers[imageIndex], framebuffers[imageIndex], swapchainExtent);
	submitCommands(commandBuffers[imageIndex], imageAvailable, renderFinished, graphicsQueue, fences[imageIndex]);

	recordComputeCommands(computeCommandBuffers[imageIndex]);
	submitComputeCommands(computeCommandBuffers[imageIndex], renderFinished, computeFinished, graphicsQueue, fences[imageIndex]);

	recordUICommands(presentCommandBuffers[imageIndex], framebuffers[imageIndex], swapchainExtent);
	submitUICommands(presentCommandBuffers[imageIndex], computeFinished, readyToPresent, graphicsQueue, fences[imageIndex]);

	presentImage(graphicsQueue, readyToPresent, swapchain, imageIndex);

	// Update Texture after recording all rendering commands
	if (gameState->materialUpdateInfo.updateTexture || gameState->materialUpdateInfo.updateColor)
	{
		changeMaterialTexture(logicalDevice, physicalDevice, graphicsQueue, gameState);
		gameState->materialUpdateInfo.textureFilepath = "";
	}

	//recordCommands(commandBuffers[imageIndex], framebuffers[imageIndex], swapchainExtent);
	//submitCommands(commandBuffers[imageIndex], imageAvailable, renderFinished, graphicsQueue, fences[imageIndex]);

	//presentImage(graphicsQueue, renderFinished, swapchain, imageIndex);
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

void VulkanEngine::createDepthPrePass(VkDevice& logicalDevice, VkRenderPass& renderPass, const VkFormat& depthFormat)
{
	// Only the depth buffer
	VkAttachmentDescription attachments[1]{};
	attachments[0].format = depthFormat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkAttachmentReference depthAttachment{};
	depthAttachment.attachment = 0;
	depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpasses[1]{};
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = 0;
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

void VulkanEngine::createPresentRenderPass(VkDevice& logicalDevice, VkRenderPass& renderPass, const VkFormat& format)
{
	VkAttachmentDescription attachments[1]{};
	attachments[0].format = format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// The out location of the fragment shader
	// 0 is the color value for the framebuffer
	VkAttachmentReference colorAttachment{};
	colorAttachment.attachment = 0;
	colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpasses[1]{};
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = 1;
	subpasses[0].pColorAttachments = &colorAttachment;

	VkRenderPassCreateInfo passInfo{};
	passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	passInfo.attachmentCount = static_cast<u32>(sizeof(attachments) / sizeof(attachments[0]));
	passInfo.pAttachments = attachments;
	passInfo.subpassCount = static_cast<u32>(sizeof(subpasses) / sizeof(subpasses[0]));
	passInfo.pSubpasses = subpasses;

	if (vkCreateRenderPass(logicalDevice, &passInfo, nullptr, &renderPass) != VK_SUCCESS)
		throw std::runtime_error("Failed to create render pass");
}

void VulkanEngine::createShadingRenderPass(VkDevice& logicalDevice, VkRenderPass& renderPass, VkFormat& format, const VkFormat& depthFormat)
{
	VkAttachmentDescription attachments[1]{};
	// Framebuffer
	attachments[0].format = format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

	// The out location of the fragment shader
	// 0 is the color value for the framebuffer
	VkAttachmentReference colorAttachment{};
	colorAttachment.attachment = 0;
	colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpasses[1]{};
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = 1;
	subpasses[0].pColorAttachments = &colorAttachment;

	VkRenderPassCreateInfo passInfo{};
	passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	passInfo.attachmentCount = static_cast<u32>(sizeof(attachments) / sizeof(attachments[0]));
	passInfo.pAttachments = attachments;
	passInfo.subpassCount = static_cast<u32>(sizeof(subpasses) / sizeof(subpasses[0]));
	passInfo.pSubpasses = subpasses;

	if (vkCreateRenderPass(logicalDevice, &passInfo, nullptr, &renderPass) != VK_SUCCESS)
		throw std::runtime_error("Failed to create render pass");
}

void VulkanEngine::createShadowRenderPass(VkDevice& logicalDevice, VkRenderPass& renderPass, const VkFormat& depthFormat)
{
	// Only the depth buffer
	VkAttachmentDescription attachments[1]{};
	attachments[0].format = depthFormat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentReference depthAttachment{};
	depthAttachment.attachment = 0;
	depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpasses[1]{};
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = 0;
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

void VulkanEngine::createGeometryRenderPass(VkDevice& logicalDevice, VkRenderPass& renderPass, VkFormat format, const VkFormat& depthFormat)
{
	std::vector<VkAttachmentDescription> attachments(6);
	// G-buffers
	for (u32 i = 0; i < attachments.size(); i++)
	{
		attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		if (i == attachments.size() - 1)
		{
			// Depth buffer
			attachments[i].format = depthFormat;
			attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[i].initialLayout = VK_IMAGE_LAYOUT_GENERAL;
			attachments[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else
		{
			attachments[i].format = format;
			attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
	}

	// The out location of the fragment shader
	// 0 is GBuffer0
	// 1 is GBuffer1
	// 2 is GBuffer2
	// 3 is GBuffer3
	// 4 is GBuffer4
	std::vector<VkAttachmentReference> colorAttachments(attachments.size() - 1);
	for (u32 i = 0; i < colorAttachments.size(); i++)
	{
		colorAttachments[i].attachment = i;
		colorAttachments[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	// The last one is the depth value for the depth buffer
	VkAttachmentReference depthAttachment{};
	depthAttachment.attachment = attachments.size() - 1;
	depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpasses[1]{};
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = static_cast<u32>(colorAttachments.size());
	subpasses[0].pColorAttachments = colorAttachments.data();
	subpasses[0].pDepthStencilAttachment = &depthAttachment;

	// Attachments layout transitions
	std::vector<VkSubpassDependency> subpassDependencies(2);
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo passInfo{};
	passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	passInfo.attachmentCount = static_cast<u32>(attachments.size());
	passInfo.pAttachments = attachments.data();
	passInfo.subpassCount = static_cast<u32>(sizeof(subpasses) / sizeof(subpasses[0]));
	passInfo.pSubpasses = subpasses;
	passInfo.dependencyCount = static_cast<u32>(subpassDependencies.size());
	passInfo.pDependencies = subpassDependencies.data();

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

void VulkanEngine::createDepthBuffer(VkDevice& logicalDevice, VmaAllocator& vmaAllocator, const VkExtent2D& extent)
{
	depthImage =  WillEngine::VulkanUtil::createImage(logicalDevice, vmaAllocator, depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, extent.width,
		extent.height, 1);

	// Create depth buffer image view
	WillEngine::VulkanUtil::createImageView(logicalDevice, depthImage.image, depthImageView, 1, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VulkanEngine::createOffscreenAttachments(VkDevice& logicalDevice, const VkExtent2D& extent)
{
	// The back buffer
	// Create offscreen framebuffer attachments
	WillEngine::VulkanUtil::createFramebufferAttachment(logicalDevice, vmaAllocator, VK_FORMAT_R16G16B16A16_SFLOAT, extent, offscreenFramebuffer.GBuffer0);
	WillEngine::VulkanUtil::createFramebufferAttachment(logicalDevice, vmaAllocator, VK_FORMAT_R16G16B16A16_SFLOAT, extent, offscreenFramebuffer.GBuffer1);
	WillEngine::VulkanUtil::createFramebufferAttachment(logicalDevice, vmaAllocator, VK_FORMAT_R16G16B16A16_SFLOAT, extent, offscreenFramebuffer.GBuffer2);
	WillEngine::VulkanUtil::createFramebufferAttachment(logicalDevice, vmaAllocator, VK_FORMAT_R16G16B16A16_SFLOAT, extent, offscreenFramebuffer.GBuffer3);
	WillEngine::VulkanUtil::createFramebufferAttachment(logicalDevice, vmaAllocator, VK_FORMAT_R16G16B16A16_SFLOAT, extent, offscreenFramebuffer.GBuffer4);

	VkImageView attachments[6]{};
	attachments[0] = offscreenFramebuffer.GBuffer0.imageView;
	attachments[1] = offscreenFramebuffer.GBuffer1.imageView;
	attachments[2] = offscreenFramebuffer.GBuffer2.imageView;
	attachments[3] = offscreenFramebuffer.GBuffer3.imageView;
	attachments[4] = offscreenFramebuffer.GBuffer4.imageView;
	attachments[5] = depthImageView;

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = geometryRenderPass;
	framebufferInfo.attachmentCount = static_cast<u32>(sizeof(attachments) / sizeof(attachments[0]));
	framebufferInfo.pAttachments = attachments;
	framebufferInfo.width = extent.width;
	framebufferInfo.height = extent.height;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &offscreenFramebuffer.framebuffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to create framebuffer");
}

void VulkanEngine::createSwapchainFramebuffer(VkDevice& logicalDevice, std::vector<VkImageView>& swapchainImageViews, std::vector<VkFramebuffer>& framebuffers, 
	VulkanFramebuffer& offscreenFramebuffer, VkRenderPass& geometryRenderPass, VkRenderPass& renderPass, VkImageView& depthImageView, VkExtent2D extent)
{
	assert(framebuffers.empty());

	// The present framebuffer
	for (u32 i = 0; i < swapchainImageViews.size(); i++)
	{
		VkImageView attachments[1]{};
		attachments[0] = swapchainImageViews[i];

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<u32>(sizeof(attachments) / sizeof(attachments[0]));
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;

		VkFramebuffer framebuffer = VK_NULL_HANDLE;

		if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS)
			throw std::runtime_error("Failed to create framebuffer");

		framebuffers.emplace_back(std::move(framebuffer));
	}

	assert(swapchainImageViews.size() == framebuffers.size());
}

void VulkanEngine::recreateSwapchainFramebuffer(GLFWwindow* window, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkQueue graphicsQueue)
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
	if (oldExtent.width != swapchainExtent.width || oldExtent.height != swapchainExtent.height || sceneExtentChanged)
		extentChanged = true;

	if (oldFormat != swapchainImageFormat)
		formatChanged = true;

	if (formatChanged)
	{
		createDepthPrePass(logicalDevice, depthRenderPass, depthFormat);
		createGeometryRenderPass(logicalDevice, geometryRenderPass, swapchainImageFormat, depthFormat);
		createShadingRenderPass(logicalDevice, shadingRenderPass, swapchainImageFormat, depthFormat);
		createPresentRenderPass(logicalDevice, presentRenderPass, swapchainImageFormat);
	}

	if (extentChanged)
	{
		// Destroy the old depth buffer
		vkDestroyImageView(logicalDevice, depthImageView, nullptr);
		vmaDestroyImage(vmaAllocator, depthImage.image, depthImage.allocation);

		// Create a new depth buffer
		//createDepthBuffer(logicalDevice, vmaAllocator, swapchainExtent);
		createDepthBuffer(logicalDevice, vmaAllocator, sceneExtent);

		// Shading
		vkDestroyImageView(logicalDevice, shadingImageView, nullptr);
		vmaDestroyImage(vmaAllocator, shadingImage.image, shadingImage.allocation);

		// Computed
		vkDestroyImageView(logicalDevice, postProcessedImageView, nullptr);
		vmaDestroyImage(vmaAllocator, postProcessedImage.image, nullptr);

		vkFreeDescriptorSets(logicalDevice, descriptorPool, 1, &gameState->graphicsState.renderedImage);
		vkDestroyDescriptorSetLayout(logicalDevice, gameState->graphicsState.renderedImageLayout, nullptr);
		vkFreeDescriptorSets(logicalDevice, vulkanGui->getDescriptorPool(), 1, &gameState->graphicsState.renderedImage_ImGui);

		vkFreeDescriptorSets(logicalDevice, descriptorPool, 1, &gameState->graphicsState.computedImage);
		vkDestroyDescriptorSetLayout(logicalDevice, gameState->graphicsState.computedImageLayout, nullptr);
		vkFreeDescriptorSets(logicalDevice, vulkanGui->getDescriptorPool(), 1, &gameState->graphicsState.computedImage_ImGui);
	}

	// Destroy old framebuffers
	
	for (auto framebuffer : framebuffers)
		vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);

	framebuffers.clear();

	offscreenFramebuffer.cleanUp(logicalDevice, vmaAllocator);

	vkDestroyFramebuffer(logicalDevice, depthFramebuffer, nullptr);

	vkDestroyFramebuffer(logicalDevice, shadingFramebuffer, nullptr);


	// Recreate framebuffers
	createDepthFramebuffer(logicalDevice, depthFramebuffer, depthRenderPass, sceneExtent);

	createOffscreenAttachments(logicalDevice, sceneExtent);

	WillEngine::VulkanUtil::createShadingImage(logicalDevice, vmaAllocator, swapchainImageFormat, sceneExtent, shadingImage, shadingImageView);
	createShadingFramebuffer(logicalDevice, shadingFramebuffer, shadingRenderPass, sceneExtent);

	WillEngine::VulkanUtil::createComputedImage(logicalDevice, vmaAllocator, commandPool, graphicsQueue, swapchainImageFormat, sceneExtent, postProcessedImage, postProcessedImageView);

	createSwapchainFramebuffer(logicalDevice, swapchainImageViews, framebuffers, offscreenFramebuffer, geometryRenderPass, shadingRenderPass, depthImageView, swapchainExtent);

	if (extentChanged)
	{
		// Recreate attachment descriptor sets
		initAttachmentDescriptors(logicalDevice, descriptorPool);
		initRenderedDescriptors(logicalDevice, descriptorPool);
		initComputedImageDescriptors(logicalDevice, descriptorPool);

		sceneExtentChanged = false;
	}
}

void VulkanEngine::createShadowFramebuffer(VkDevice& logicalDevice, VkFramebuffer& shadowFramebuffer, VkRenderPass& shadowRenderPass, u32 width, u32 height)
{
	VkImageView attachments[1]{};
	attachments[0] = shadowCubeMapView;

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = shadowRenderPass;
	framebufferInfo.attachmentCount = static_cast<u32>(sizeof(attachments) / sizeof(attachments[0]));
	framebufferInfo.pAttachments = attachments;
	framebufferInfo.width = width;
	framebufferInfo.height = height;
	framebufferInfo.layers = 6;

	if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &shadowFramebuffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to create framebuffer");
}

void VulkanEngine::createShadingFramebuffer(VkDevice& logicalDevice, VkFramebuffer& shadingFramebuffer, VkRenderPass& shadingRenderPass, VkExtent2D extent)
{
	VkImageView attachments[1]{};
	attachments[0] = shadingImageView;

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = shadingRenderPass;
	framebufferInfo.attachmentCount = static_cast<u32>(sizeof(attachments) / sizeof(attachments[0]));
	framebufferInfo.pAttachments = attachments;
	framebufferInfo.width = extent.width;
	framebufferInfo.height = extent.height;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &shadingFramebuffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to create framebuffer");
}

void VulkanEngine::createDepthFramebuffer(VkDevice& logicalDevice, VkFramebuffer& depthFramebuffer, VkRenderPass& depthRenderPass, VkExtent2D extent)
{
	VkImageView attachments[1]{};
	attachments[0] = depthImageView;

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = depthRenderPass;
	framebufferInfo.attachmentCount = static_cast<u32>(sizeof(attachments) / sizeof(attachments[0]));
	framebufferInfo.pAttachments = attachments;
	framebufferInfo.width = extent.width;
	framebufferInfo.height = extent.height;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &depthFramebuffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to create framebuffer");

}

void VulkanEngine::createCommandPool(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkCommandPool& commandPool)
{
	commandPool = WillEngine::VulkanUtil::createCommandPool(logicalDevice, physicalDevice, surface);
}

void VulkanEngine::createCommandBuffer(VkDevice& logicalDevice, std::vector<VkCommandBuffer>& commandBuffers, 
	std::vector<VkCommandBuffer>& computeCommandBuffers, std::vector<VkCommandBuffer>& presentCommandBuffers)
{
	commandBuffers.resize(numSwapchainImage);
	computeCommandBuffers.resize(numSwapchainImage);
	presentCommandBuffers.resize(numSwapchainImage);

	for (u32 i = 0; i < commandBuffers.size(); i++)
	{
		commandBuffers[i] = WillEngine::VulkanUtil::createCommandBuffer(logicalDevice, commandPool);
		computeCommandBuffers[i] = WillEngine::VulkanUtil::createCommandBuffer(logicalDevice, commandPool);
		presentCommandBuffers[i] = WillEngine::VulkanUtil::createCommandBuffer(logicalDevice, commandPool);
	}
}

void VulkanEngine::createSemaphore(VkDevice& logicalDevice, VkSemaphore& waitImageAvailable, VkSemaphore& signalRenderFinish, VkSemaphore& signalReadyToPresent,
	VkSemaphore& signalComputeFinished)
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &waitImageAvailable) != VK_SUCCESS)
		throw std::runtime_error("Failed to create wait image availabe semaphore");

	if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &signalRenderFinish) != VK_SUCCESS)
		throw std::runtime_error("Failed to create signal render finish semaphore");

	if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &signalReadyToPresent) != VK_SUCCESS)
		throw std::runtime_error("Failed to create signal ready to present semaphore");

	if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &signalComputeFinished) != VK_SUCCESS)
		throw std::runtime_error("Failed to create signal ready to present semaphore");
}

void VulkanEngine::createFence(VkDevice& logicalDevice, std::vector<VkFence>& fences, VkFenceCreateFlagBits flag)
{
	fences.resize(numSwapchainImage);

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

void VulkanEngine::initShadowMapDescriptors(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool)
{
	WillEngine::VulkanUtil::createDescriptorSetLayout(logicalDevice, shadowMapDescriptorSetLayouts, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT, 3, 1);

	WillEngine::VulkanUtil::allocDescriptorSet(logicalDevice, descriptorPool, shadowMapDescriptorSetLayouts, shadowMapDescriptorSets);

	std::vector<VkImageView> imageViews = { shadowCubeMapView };

	WillEngine::VulkanUtil::writeDescriptorSetImage(logicalDevice, shadowMapDescriptorSets, &shadowSampler, imageViews.data(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, imageViews.size());
}

void VulkanEngine::initAttachmentDescriptors(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool)
{
	// Descriptor sets for ImGui UI
	{
		offscreenFramebuffer.GBuffer0.imguiTextureDescriptorSet = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(attachmentSampler, offscreenFramebuffer.GBuffer0.imageView,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		offscreenFramebuffer.GBuffer1.imguiTextureDescriptorSet = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(attachmentSampler, offscreenFramebuffer.GBuffer1.imageView,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		offscreenFramebuffer.GBuffer2.imguiTextureDescriptorSet = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(attachmentSampler, offscreenFramebuffer.GBuffer2.imageView,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		offscreenFramebuffer.GBuffer3.imguiTextureDescriptorSet = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(attachmentSampler, offscreenFramebuffer.GBuffer3.imageView,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		offscreenFramebuffer.GBuffer4.imguiTextureDescriptorSet = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(attachmentSampler, offscreenFramebuffer.GBuffer4.imageView,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	WillEngine::VulkanUtil::createDescriptorSetLayout(logicalDevice, attachmentDescriptorSetLayouts, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT, 2, 5);

	WillEngine::VulkanUtil::allocDescriptorSet(logicalDevice, descriptorPool, attachmentDescriptorSetLayouts, attachmentDescriptorSets);

	std::vector<VkImageView> imageViews = { offscreenFramebuffer.GBuffer0.imageView, offscreenFramebuffer.GBuffer1.imageView, offscreenFramebuffer.GBuffer2.imageView,
		offscreenFramebuffer.GBuffer3.imageView, offscreenFramebuffer.GBuffer4.imageView };

	WillEngine::VulkanUtil::writeDescriptorSetImage(logicalDevice, attachmentDescriptorSets, &attachmentSampler, imageViews.data(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, imageViews.size());
}

void VulkanEngine::initRenderedDescriptors(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool)
{
	//WillEngine::VulkanUtil::createDescriptorSetLayout(logicalDevice, gameState->graphicsState.renderedImageLayout, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	//	VK_SHADER_STAGE_COMPUTE_BIT, 0, 1);

	//WillEngine::VulkanUtil::allocDescriptorSet(logicalDevice, descriptorPool, gameState->graphicsState.renderedImageLayout, gameState->graphicsState.renderedImage);

	//std::vector<VkImageView> imageViews = { shadingImageView };

	//WillEngine::VulkanUtil::writeDescriptorSetImage(logicalDevice, gameState->graphicsState.renderedImage, &defaultSampler, imageViews.data(),
	//	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, 1);

	WillEngine::VulkanUtil::createDescriptorSetLayout(logicalDevice, gameState->graphicsState.renderedImageLayout, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		VK_SHADER_STAGE_COMPUTE_BIT, 0, 1);

	WillEngine::VulkanUtil::allocDescriptorSet(logicalDevice, descriptorPool, gameState->graphicsState.renderedImageLayout, gameState->graphicsState.renderedImage);

	std::vector<VkImageView> imageViews = { shadingImageView };

	WillEngine::VulkanUtil::writeDescriptorSetImage(logicalDevice, gameState->graphicsState.renderedImage, &defaultSampler, imageViews.data(),
		VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, 1);

	gameState->graphicsState.renderedImage_ImGui = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(attachmentSampler, shadingImageView, VK_IMAGE_LAYOUT_GENERAL);
}

void VulkanEngine::initComputedImageDescriptors(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool)
{
	WillEngine::VulkanUtil::createDescriptorSetLayout(logicalDevice, gameState->graphicsState.computedImageLayout, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1, 1);

	WillEngine::VulkanUtil::allocDescriptorSet(logicalDevice, descriptorPool, gameState->graphicsState.computedImageLayout, gameState->graphicsState.computedImage);

	std::vector<VkImageView> imageViews = { postProcessedImageView };

	WillEngine::VulkanUtil::writeDescriptorSetImage(logicalDevice, gameState->graphicsState.computedImage, &defaultSampler, imageViews.data(), VK_IMAGE_LAYOUT_GENERAL,
		VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, 1);

	//gameState->graphicsState.computedImage_ImGui = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(defaultSampler, postProcessedImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	gameState->graphicsState.computedImage_ImGui = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(defaultSampler, postProcessedImageView, VK_IMAGE_LAYOUT_GENERAL);
}

void VulkanEngine::initGeometryPipeline(VkDevice& logicalDevice)
{
	// Set up shader modules
	WillEngine::VulkanUtil::initGeometryShaderModule(logicalDevice, geometryVertShader, geometryFragShader);

	// Create pipeline and pipeline layout
	VkDescriptorSetLayout layouts[] = { sceneDescriptorSetLayout, textureDescriptorSetLayout };
	u32 descriptorSetLayoutSize = sizeof(layouts) / sizeof(layouts[0]);

	VkPushConstantRange pushConstants[1];
	// Push constant object for model matrix to be used in vertex shader
	pushConstants[0].offset = 0;
	pushConstants[0].size = sizeof(mat4);
	pushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	u32 pushConstantCount = sizeof(pushConstants) / sizeof(pushConstants[0]);

	// Create deferred pipeline layout with our just created push constant
	WillEngine::VulkanUtil::createPipelineLayout(logicalDevice, geometryPipelineLayout,
		descriptorSetLayoutSize, layouts, pushConstantCount, pushConstants);

	// Create deferred pipeline
	WillEngine::VulkanUtil::createGeometryPipeline(logicalDevice, geometryPipeline, geometryPipelineLayout,
		geometryRenderPass, geometryVertShader, geometryFragShader, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, sceneExtent);
}

void VulkanEngine::initDepthPipeline(VkDevice& logicalDevice)
{
	VkDescriptorSetLayout depthLayouts[] = { sceneDescriptorSetLayout };
	u32 depthDescriptorSetLayoutSize = sizeof(depthLayouts) / sizeof(depthLayouts[0]);

	createDepthFramebuffer(logicalDevice, depthFramebuffer, depthRenderPass, sceneExtent);

	WillEngine::VulkanUtil::initDepthShaderModule(logicalDevice, depthVertShader, depthFragShader);

	VkPushConstantRange pushConstants[1];
	// Push constant object for model matrix to be used in vertex shader
	pushConstants[0].offset = 0;
	pushConstants[0].size = sizeof(mat4);
	pushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	u32 pushConstantCount = sizeof(pushConstants) / sizeof(pushConstants[0]);

	WillEngine::VulkanUtil::createPipelineLayout(logicalDevice, depthPipelineLayout, depthDescriptorSetLayoutSize, depthLayouts, pushConstantCount, pushConstants);

	WillEngine::VulkanUtil::createDepthPipeline(logicalDevice, depthPipeline, depthPipelineLayout, depthRenderPass, depthVertShader, depthFragShader,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, sceneExtent);
}

void VulkanEngine::initShadowPipeline(VkDevice& logicalDevice)
{
	// Create an image, imageview and sampler for point light's cube shadow map
	shadowCubeMap = WillEngine::VulkanUtil::createImageWithFlags(logicalDevice, vmaAllocator, shadowDepthFormat,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
		1024, 1024, 1, 6);

	WillEngine::VulkanUtil::createDepthImageView(logicalDevice, shadowCubeMap.image, shadowCubeMapView, 6, shadowDepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	WillEngine::VulkanUtil::createDepthSampler(logicalDevice, shadowSampler);

	// Shadow Framebuffer
	createShadowFramebuffer(logicalDevice, shadowFramebuffer, shadowRenderPass, 1024, 1024);

	VkDescriptorSetLayout layout[] = { lightMatrixDescriptorSetLayout, lightDescriptorSetLayout };
	u32 layoutSize = sizeof(layout) / sizeof(layout[0]);

	VkPushConstantRange pushConstant[1];
	pushConstant[0].offset = 0;
	pushConstant[0].size = sizeof(mat4);
	pushConstant[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	u32 pushConstantCount = sizeof(pushConstant) / sizeof(pushConstant[0]);

	WillEngine::VulkanUtil::initShadowShaderModule(logicalDevice, shadowVertShader, shadowGeomShader, shadowFragShader);

	WillEngine::VulkanUtil::createPipelineLayout(logicalDevice, shadowPipelineLayout, layoutSize, layout, pushConstantCount, pushConstant);

	WillEngine::VulkanUtil::createShadowPipeline(logicalDevice, shadowPipeline, shadowPipelineLayout, shadowRenderPass, shadowVertShader, shadowGeomShader,
		shadowFragShader, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 1024, 1024);
}

void VulkanEngine::initShadingPipeline(VkDevice& logicalDevice)
{
	// Initialise frame buffer attachments as descriptors
	initAttachmentDescriptors(logicalDevice, descriptorPool);

	initShadowMapDescriptors(logicalDevice, descriptorPool);

	WillEngine::VulkanUtil::initShadingShaderModule(logicalDevice, shadingVertShader, shadingFragShader);

	VkDescriptorSetLayout layout[] = { lightDescriptorSetLayout, cameraDescriptorSetLayout, attachmentDescriptorSetLayouts, shadowMapDescriptorSetLayouts };
	u32 layoutSize = sizeof(layout) / sizeof(layout[0]);

	WillEngine::VulkanUtil::createPipelineLayout(logicalDevice, shadingPipelineLayout, layoutSize, layout, 0, nullptr);

	WillEngine::VulkanUtil::createShadingPipeline(logicalDevice, shadingPipeline, shadingPipelineLayout, shadingRenderPass, shadingVertShader, shadingFragShader,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, sceneExtent);
}

void VulkanEngine::initBloomDownscalePipeline(VkDevice& logicalDevice)
{
	
	WillEngine::VulkanUtil::initBloomDownscaleShaderModule(logicalDevice, bloomDownscaleCompShader);
	
	VkDescriptorSetLayout layout[] = {gameState->graphicsState.renderedImageLayout, gameState->graphicsState.computedImageLayout};
	u32 layoutSize = sizeof(layout) / sizeof(layout[0]);

	WillEngine::VulkanUtil::createPipelineLayout(logicalDevice, bloomDownscalePipelineLayout, layoutSize, layout, 0, nullptr);

	WillEngine::VulkanUtil::createBloomDownscalePipeline(logicalDevice, bloomDownscalePipeline, bloomDownscalePipelineLayout, bloomDownscaleCompShader);

}

void VulkanEngine::initGui(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkQueue& queue,
	VkSurfaceKHR& surface)
{
	vulkanGui = new VulkanGui();

	std::optional<u32> graphicsFamilyIndicies = WillEngine::VulkanUtil::findQueueFamilies(physicalDevice, VK_QUEUE_GRAPHICS_BIT, VK_NULL_HANDLE);

	if (!graphicsFamilyIndicies.has_value())
		std::runtime_error("Failed to get graphics queue family index");

	vulkanGui->init(window, instance, logicalDevice, physicalDevice, surface, graphicsFamilyIndicies.value(), commandPool, descriptorPool, numSwapchainImage, shadingRenderPass,
		swapchainExtent);
}

void VulkanEngine::updateSceneUniform(Camera* camera)
{
	// Update camera 
	sceneMatrix.viewMatrix = camera->getCameraMatrix();
	sceneMatrix.projectionMatrix = camera->getProjectionMatrix(sceneExtent.width, sceneExtent.height);
}

void VulkanEngine::updateLightUniform(Camera* camera)
{
	// Update Light's Position
	for (auto light : gameState->graphicsResources.lights)
	{
		light->lightUniform.transformedPosition = vec4(light->position.x, light->position.y, light->position.z, 1);
	}
}

void VulkanEngine::recordCommands(VkCommandBuffer& commandBuffer, VkFramebuffer& framebuffer, VkExtent2D& extent)
{
	// Begin command buffer
	VkCommandBufferBeginInfo commandBeginInfo{};
	commandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(commandBuffer, &commandBeginInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to begin command buffer");

	// Update uniform buffers
	vkCmdUpdateBuffer(commandBuffer, sceneUniformBuffer.buffer, 0, sizeof(sceneMatrix), &sceneMatrix);

	// Update light uniform buffers
	vkCmdUpdateBuffer(commandBuffer, lightUniformBuffer.buffer, 0, sizeof(gameState->graphicsResources.lights[0]->lightUniform), &gameState->graphicsResources.lights[0]->lightUniform);

	vec4 cameraPosition = vec4(camera->position, 1);

	// Update camera uniform buffers
	vkCmdUpdateBuffer(commandBuffer, cameraUniformBuffer.buffer, 0, sizeof(vec4), &cameraPosition);

	// Set dynamic viewport and scissor
	{
		VkViewport viewport = WillEngine::VulkanUtil::getViewport(sceneExtent);
		VkRect2D scissor = WillEngine::VulkanUtil::getScissor(sceneExtent);
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	// ========================================================
	// Recording all the rendering command

	depthPrePasses(commandBuffer, sceneExtent);

	// Deferred rendering pass
	geometryPasses(commandBuffer, sceneExtent);

	// Shadow pass
	if (gameState->graphicsResources.lights[0]->shouldRenderShadow())
	{
		shadowPasses(commandBuffer);
		gameState->graphicsResources.lights[0]->shadowRendered();
	}

	// Shading
	shadingPasses(commandBuffer, shadingRenderPass, shadingFramebuffer, sceneExtent);

	// UI rendering pass
	//UIPasses(commandBuffer, presentRenderPass, framebuffer, extent);

	// ========================================================

	// End command buffer
	vkEndCommandBuffer(commandBuffer);
}

void VulkanEngine::depthPrePasses(VkCommandBuffer& commandBuffer, VkExtent2D extent)
{
	VkClearValue clearValue[1];
	clearValue[0].depthStencil.depth = 1.0f;

	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = depthRenderPass;
	renderPassBeginInfo.framebuffer = depthFramebuffer;
	renderPassBeginInfo.renderArea.extent = sceneExtent;
	renderPassBeginInfo.clearValueCount = static_cast<u32>(sizeof(clearValue) / sizeof(clearValue[0]));
	renderPassBeginInfo.pClearValues = clearValue;

	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Bind pipeline
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, depthPipeline);

	// Bind Scene Uniform Buffer
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, depthPipelineLayout, 0, 1, &sceneDescriptorSet, 0, nullptr);

	for (auto mesh : gameState->graphicsResources.meshes)
	{
		VkBuffer buffers[3] = { mesh->positionBuffer.buffer, mesh->normalBuffer.buffer, mesh->uvBuffer.buffer };

		VkDeviceSize offsets[3]{};

		// Bind buffers
		vkCmdBindVertexBuffers(commandBuffer, 0, 3, buffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer, mesh->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		// Push constant for model matrix
		mesh->updateForPushConstant();
		vkCmdPushConstants(commandBuffer, depthPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mesh->pushConstant), &mesh->pushConstant);

		vkCmdDrawIndexed(commandBuffer, static_cast<u32>(mesh->indiciesSize), 3, 0, 0, 0);
	}

	vkCmdEndRenderPass(commandBuffer);
}

void VulkanEngine::geometryPasses(VkCommandBuffer& commandBuffer, VkExtent2D extent)
{
	VkClearValue clearValue[5];
	// Clear attachments
	// We're not clearing depth as we're using compare_equal to the depth buffer from depth pre-pass
	for (u32 i = 0; i < 5; i++)
	{
		clearValue[i].color.float32[0] = 0.0f;
		clearValue[i].color.float32[1] = 0.0f;
		clearValue[i].color.float32[2] = 0.0f;
		clearValue[i].color.float32[3] = 1.0f;
	}

	// Begin Render Pass
	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = geometryRenderPass;
	renderPassBeginInfo.framebuffer = offscreenFramebuffer.framebuffer;
	renderPassBeginInfo.renderArea.extent = sceneExtent;
	renderPassBeginInfo.clearValueCount = static_cast<u32>(sizeof(clearValue) / sizeof(clearValue[0]));
	renderPassBeginInfo.pClearValues = clearValue;

	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Bind default pipeline
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, geometryPipeline);

	// Bind Scene Uniform Buffer
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, geometryPipelineLayout, 0, 1, &sceneDescriptorSet, 0, nullptr);

	for (auto mesh : gameState->graphicsResources.meshes)
	{
		VkBuffer buffers[3] = { mesh->positionBuffer.buffer, mesh->normalBuffer.buffer, mesh->uvBuffer.buffer };

		VkDeviceSize offsets[3]{};

		// Bind buffers
		vkCmdBindVertexBuffers(commandBuffer, 0, 3, buffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer, mesh->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		// Bind Texture
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, geometryPipelineLayout, 1, 1, &gameState->graphicsResources.materials[mesh->materialIndex]->textureDescriptorSet, 0, nullptr);

		// Push constant for model matrix
		mesh->updateForPushConstant();
		vkCmdPushConstants(commandBuffer, geometryPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mesh->pushConstant), &mesh->pushConstant);

		vkCmdDrawIndexed(commandBuffer, static_cast<u32>(mesh->indiciesSize), 3, 0, 0, 0);
	}

	// End Render Pass
	vkCmdEndRenderPass(commandBuffer);
}

void VulkanEngine::shadowPasses(VkCommandBuffer& commandBuffer)
{
	// Update light matrices buffer
	vkCmdUpdateBuffer(commandBuffer, lightMatrixUniformBuffer.buffer, 0, sizeof(mat4) * 6, &gameState->graphicsResources.lights[0]->matrices);

	VkClearValue clearValue[1];
	// Clear Depth
	clearValue[0].depthStencil.depth = 1.0f;

	// Begin Render Pass
	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = shadowRenderPass;
	renderPassBeginInfo.framebuffer = shadowFramebuffer;
	renderPassBeginInfo.renderArea.extent.width = 1024;
	renderPassBeginInfo.renderArea.extent.height = 1024;
	renderPassBeginInfo.clearValueCount = static_cast<u32>(sizeof(clearValue) / sizeof(clearValue[0]));
	renderPassBeginInfo.pClearValues = clearValue;

	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Bind shadow pipeline
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipeline);

	// Bind Light Uniform Buffer
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipelineLayout, 1, 1, &lightDescriptorSet, 0, nullptr);

	// Bind light matrices
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipelineLayout, 0, 1, &lightMatrixDescriptorSet, 0, nullptr);

	for (auto mesh : gameState->graphicsResources.meshes)
	{
		VkBuffer buffers[3] = { mesh->positionBuffer.buffer, mesh->normalBuffer.buffer, mesh->uvBuffer.buffer };

		VkDeviceSize offsets[3]{};

		// Bind buffers
		vkCmdBindVertexBuffers(commandBuffer, 0, 3, buffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer, mesh->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		// Push constant for model matrix
		mesh->updateForPushConstant();
		vkCmdPushConstants(commandBuffer, geometryPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mesh->pushConstant), &mesh->pushConstant);

		vkCmdDrawIndexed(commandBuffer, static_cast<u32>(mesh->indiciesSize), 3, 0, 0, 0);
	}

	vkCmdEndRenderPass(commandBuffer);
}

void VulkanEngine::shadingPasses(VkCommandBuffer& commandBuffer, VkRenderPass& renderPass, VkFramebuffer& framebuffer, VkExtent2D extent)
{
	// Combine offscreen framebuffer
	VkClearValue clearValue[2];
	// Clear color
	clearValue[0].color.float32[0] = 0.5f;
	clearValue[0].color.float32[1] = 0.5f;
	clearValue[0].color.float32[2] = 0.5f;
	clearValue[0].color.float32[3] = 1.0f;
	// Clear Depth
	clearValue[1].depthStencil.depth = 1.0f;

	VkRenderPassBeginInfo passBeginInfo{};
	passBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	passBeginInfo.renderPass = renderPass;
	passBeginInfo.framebuffer = framebuffer;
	passBeginInfo.renderArea.extent = sceneExtent;
	passBeginInfo.clearValueCount = static_cast<u32>(sizeof(clearValue) / sizeof(clearValue[0]));
	passBeginInfo.pClearValues = clearValue;

	vkCmdBeginRenderPass(commandBuffer, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadingPipeline);

	if (gameState->graphicsResources.meshes.size() < 1)
	{
		vkCmdEndRenderPass(commandBuffer);
		return;
	}

	// Bind Light Uniform Buffer
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadingPipelineLayout, 0, 1, &lightDescriptorSet, 0, nullptr);

	// Bind Camera Uniform Buffer
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadingPipelineLayout, 1, 1, &cameraDescriptorSet, 0, nullptr);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadingPipelineLayout, 2, 1, &attachmentDescriptorSets, 0, nullptr);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadingPipelineLayout, 3, 1, &shadowMapDescriptorSets, 0, nullptr);

	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(commandBuffer);
}

void VulkanEngine::UIPasses(VkCommandBuffer& commandBuffer, VkRenderPass& renderPass, VkFramebuffer& framebuffer, VkExtent2D extent)
{
	// Combine offscreen framebuffer
	VkClearValue clearValue[1];
	// Clear color
	clearValue[0].color.float32[0] = 0.5f;
	clearValue[0].color.float32[1] = 0.5f;
	clearValue[0].color.float32[2] = 0.5f;
	clearValue[0].color.float32[3] = 1.0f;

	VkRenderPassBeginInfo passBeginInfo{};
	passBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	passBeginInfo.renderPass = renderPass;
	passBeginInfo.framebuffer = framebuffer;
	passBeginInfo.renderArea.extent = extent;
	passBeginInfo.clearValueCount = static_cast<u32>(sizeof(clearValue) / sizeof(clearValue[0]));
	passBeginInfo.pClearValues = clearValue;

	vkCmdBeginRenderPass(commandBuffer, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	// ImGui UI rendering
	vulkanGui->renderUI(commandBuffer, extent);

	vkCmdEndRenderPass(commandBuffer);
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

	//vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence);
	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
}

void VulkanEngine::recordComputeCommands(VkCommandBuffer& commandBuffer)
{
	// Begin command buffer
	VkCommandBufferBeginInfo commandBeginInfo{};
	commandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(commandBuffer, &commandBeginInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to begin command buffer");

	// Bind Pipeline
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, bloomDownscalePipeline);

	// Bind Descriptor sets
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, bloomDownscalePipelineLayout, 0, 1, &gameState->graphicsState.renderedImage, 0, nullptr);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, bloomDownscalePipelineLayout, 1, 1, &gameState->graphicsState.computedImage, 0, nullptr);

	// Dispatch compute job.
	vkCmdDispatch(commandBuffer, sceneExtent.width / 16, sceneExtent.height / 16, 1);

	vkEndCommandBuffer(commandBuffer);
}

void VulkanEngine::submitComputeCommands(VkCommandBuffer& commandBuffer, VkSemaphore& waitSemaphore, VkSemaphore& signalSemaphore, VkQueue& queue, VkFence& fence)
{
	VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &waitSemaphore;
	submitInfo.pWaitDstStageMask = &dstStageMask;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &signalSemaphore;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
}

void VulkanEngine::recordUICommands(VkCommandBuffer& commandBuffer, VkFramebuffer& framebuffer, VkExtent2D& extent)
{
	// Begin command buffer
	VkCommandBufferBeginInfo commandBeginInfo{};
	commandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(commandBuffer, &commandBeginInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to begin command buffer");

	UIPasses(commandBuffer, presentRenderPass, framebuffer, extent);

	vkEndCommandBuffer(commandBuffer);
}

void VulkanEngine::submitUICommands(VkCommandBuffer& commandBuffer, VkSemaphore& waitSemaphore, VkSemaphore& signalSemaphore, VkQueue& queue, VkFence& fence)
{
	VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &waitSemaphore;
	submitInfo.pWaitDstStageMask = &dstStageMask;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &signalSemaphore;

	vkQueueSubmit(queue, 1, &submitInfo, fence);
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

void VulkanEngine::changeMaterialTexture(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue, GameState* gameState)
{
	vkDeviceWaitIdle(logicalDevice);

	Material* currentMaterial = gameState->graphicsResources.materials[gameState->materialUpdateInfo.materialIndex];
	u32& textureIndex = gameState->materialUpdateInfo.textureIndex;
	TextureDescriptorSet& currentTexture = currentMaterial->brdfTextures[textureIndex];

	if (gameState->materialUpdateInfo.updateTexture)
	{
		// Load the texture first, if we cannot read the texture, we proceed to changing the color
		Image* textureImage = new Image();
		currentTexture.textureImage = textureImage;

		// Don't update the texture path if it's the same one or an empty string
		if (gameState->materialUpdateInfo.textureFilepath.compare("") != 0 &&
			(currentTexture.texture_path.size() < 1 ||
				currentTexture.texture_path.compare(gameState->materialUpdateInfo.textureFilepath) != 0))
		{
			currentTexture.texture_path = gameState->materialUpdateInfo.textureFilepath;
		}

		currentTexture.textureImage->readImage(currentTexture.texture_path.c_str(), currentTexture.width, currentTexture.height, currentTexture.numChannels);

		// If we cannot load the texture, create a color texture
		if (currentTexture.textureImage->data == NULL)
		{
			delete textureImage;
			currentTexture.texture_path = "";
			gameState->materialUpdateInfo.updateColor = true;
		}
		
		gameState->materialUpdateInfo.updateTexture = false;
	}

	if (gameState->materialUpdateInfo.updateColor)
	{
		// Create new data
		Image* textureImage = new Image();

		currentTexture.textureImage = textureImage;

		currentTexture.width = 1;
		currentTexture.height = 1;

		switch (textureIndex)
		{
		case 0:
			currentMaterial->brdfTextures[textureIndex].textureImage->setImageColor(currentMaterial->brdfMaterialUniform.emissive);
			break;
		case 1:
			currentMaterial->brdfTextures[textureIndex].textureImage->setImageColor(currentMaterial->brdfMaterialUniform.ambient);
			break;
		case 2:
			currentMaterial->brdfTextures[textureIndex].textureImage->setImageColor(currentMaterial->brdfMaterialUniform.albedo);
			break;
		case 3:
			currentMaterial->brdfTextures[textureIndex].textureImage->setImageColor(currentMaterial->brdfMaterialUniform.metallic);
			break;
		case 4:
			currentMaterial->brdfTextures[textureIndex].textureImage->setImageColor(currentMaterial->brdfMaterialUniform.roughness);
			break;
		}

		currentTexture.has_texture = false;

		gameState->materialUpdateInfo.updateColor = false;
	}

	// Update the associated descriptor set
	currentMaterial->updateBrdfDescriptorSet(logicalDevice, physicalDevice, vmaAllocator, commandPool, descriptorPool, graphicsQueue, textureIndex);
}