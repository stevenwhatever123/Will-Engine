#include "pch.h"

#include "Core/Vulkan/VulkanEngine.h"

using namespace WillEngine;

VulkanEngine::VulkanEngine() :
	camera(nullptr),
	vmaAllocator(VK_NULL_HANDLE),
	geometryRenderPass(VK_NULL_HANDLE),
	shadingRenderPass(VK_NULL_HANDLE),
	swapchain(VK_NULL_HANDLE),
	swapchainImageFormat(),
	depthImage({ VK_NULL_HANDLE, VK_NULL_HANDLE }),
	framebuffers(),
	offscreenFramebuffer(),
	attachmentSampler(VK_NULL_HANDLE),
	commandPool(VK_NULL_HANDLE),
	geometryBuffers(),
	imageAvailable(VK_NULL_HANDLE),
	renderFinished(VK_NULL_HANDLE),
	fences(),
	descriptorPool(VK_NULL_HANDLE),
	geometryPipelineLayout(VK_NULL_HANDLE),
	geometryPipeline(VK_NULL_HANDLE),
	shadingPipelineLayout(VK_NULL_HANDLE),
	shadingPipeline(VK_NULL_HANDLE),
	sceneDescriptorSet({ VK_NULL_HANDLE, VK_NULL_HANDLE }),
	sceneUniformBuffer({ VK_NULL_HANDLE, VK_NULL_HANDLE }),
	lightDescriptorSet({ VK_NULL_HANDLE, VK_NULL_HANDLE }),
	lightUniformBuffer({ VK_NULL_HANDLE, VK_NULL_HANDLE }),
	cameraDescriptorSet({ VK_NULL_HANDLE, VK_NULL_HANDLE }),
	cameraUniformBuffer({ VK_NULL_HANDLE, VK_NULL_HANDLE }),
	textureDescriptorSetLayout(VK_NULL_HANDLE),
	attachmentDescriptorSet({ VK_NULL_HANDLE, VK_NULL_HANDLE }),
	shadowMapDescriptorSet({ VK_NULL_HANDLE, VK_NULL_HANDLE }),
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
	createCommandBuffers(logicalDevice);
	createSemaphore(logicalDevice);
	createFence(logicalDevice, fences, VK_FENCE_CREATE_SIGNALED_BIT);
	createDescriptionPool(logicalDevice);

	WillEngine::VulkanUtil::createDefaultSampler(logicalDevice, defaultSampler);
	WillEngine::VulkanUtil::createAttachmentSampler(logicalDevice, attachmentSampler);

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

	WillEngine::VulkanUtil::createShadingImage(logicalDevice, vmaAllocator, swapchainImageFormat, sceneExtent, shadingImage);
	createShadingFramebuffer(logicalDevice, shadingFramebuffer, shadingRenderPass, sceneExtent);

	//WillEngine::VulkanUtil::createComputedImage(logicalDevice, vmaAllocator, commandPool, graphicsQueue, swapchainImageFormat, sceneExtent, postProcessedImage);

	VkExtent2D sceneExtentTemp = sceneExtent;

	for (u32 i = 0; i < downSampleImages.size(); i++)
	{
		WillEngine::VulkanUtil::createComputedImage(logicalDevice, vmaAllocator, commandPool, graphicsQueue, swapchainImageFormat, sceneExtentTemp, downSampleImages[i]);

		sceneExtentTemp.width /= 2;
		sceneExtentTemp.height /= 2;
	}

	sceneExtentTemp = sceneExtent;
	for (u32 i = 0; i < upSampleImages.size(); i++)
	{
		WillEngine::VulkanUtil::createComputedImage(logicalDevice, vmaAllocator, commandPool, graphicsQueue, swapchainImageFormat, sceneExtentTemp, upSampleImages[i]);

		sceneExtentTemp.width /= 2;
		sceneExtentTemp.height /= 2;
	}

	// Create and allocate framebuffer for presenting
	createSwapchainFramebuffer(logicalDevice, swapchainImageViews, framebuffers, offscreenFramebuffer, geometryRenderPass, shadingRenderPass, depthImage.imageView, swapchainExtent);

	// Gui
	initGui(window, instance, logicalDevice, physicalDevice, graphicsQueue, surface);

	// Used in mostly all passes
	// Scene Descriptors for scene matrix with binding 0 in vertex shader
	initUniformBuffer(logicalDevice, descriptorPool, sceneUniformBuffer, sceneDescriptorSet.descriptorSet, sceneDescriptorSet.layout, 0, sizeof(CameraMatrix), VK_SHADER_STAGE_VERTEX_BIT);

	// For shading phase
	// Light Descriptors for light with binding 1 in fragment shader
	initUniformBuffer(logicalDevice, descriptorPool, lightUniformBuffer, lightDescriptorSet.descriptorSet, lightDescriptorSet.layout, 1, sizeof(LightUniform), VK_SHADER_STAGE_FRAGMENT_BIT);

	// For Shadowing mapping
	// Light Descriptor for light view projection with binding 2 in geometry shader
	initUniformBuffer(logicalDevice, descriptorPool, lightMatrixUniformBuffer, lightMatrixDescriptorSet.descriptorSet, lightMatrixDescriptorSet.layout, 2, sizeof(mat4) * 6,
		VK_SHADER_STAGE_GEOMETRY_BIT);

	// Camera View Projection
	// Camera Descriptors for camera position with binding 1 in fragment shader
	initUniformBuffer(logicalDevice, descriptorPool, cameraUniformBuffer, cameraDescriptorSet.descriptorSet, cameraDescriptorSet.layout, 1, sizeof(vec4), VK_SHADER_STAGE_FRAGMENT_BIT);

	// Phong: 4, BRDF: 5
	u32 descriptorSize = 5;

	// Texture Descriptor with binding 1 in fragment shader
	// We only need to know the layout of the descriptor
	WillEngine::VulkanUtil::createDescriptorSetLayout(logicalDevice, textureDescriptorSetLayout, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT, 1, descriptorSize);

	// Graphics Pipeline
	initDepthPipeline(logicalDevice);
	initShadowPipeline(logicalDevice);
	initGeometryPipeline(logicalDevice);
	initShadingPipeline(logicalDevice);

	// Descriptor Set for the final shaded image to be used in the UI rendering
	initRenderedDescriptors(logicalDevice, descriptorPool);

	initComputedImageDescriptors(logicalDevice, descriptorPool);

	// Compute Pipeline for bloom
	initFilterBrightPipeline(logicalDevice);
	initDownscalePipeline(logicalDevice);
	initUpscalePipeline(logicalDevice);
	initBlendColorPipeline(logicalDevice);
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
	vkFreeDescriptorSets(logicalDevice, descriptorPool, 1, &sceneDescriptorSet.descriptorSet);
	vkDestroyDescriptorSetLayout(logicalDevice, sceneDescriptorSet.layout, nullptr);
	// Light
	vmaDestroyBuffer(vmaAllocator, lightUniformBuffer.buffer, lightUniformBuffer.allocation);
	vkFreeDescriptorSets(logicalDevice, descriptorPool, 1, &lightDescriptorSet.descriptorSet);
	vkDestroyDescriptorSetLayout(logicalDevice, lightDescriptorSet.layout, nullptr);
	// Camera
	vmaDestroyBuffer(vmaAllocator, cameraUniformBuffer.buffer, cameraUniformBuffer.allocation);
	vkFreeDescriptorSets(logicalDevice, descriptorPool, 1, &cameraDescriptorSet.descriptorSet);
	vkDestroyDescriptorSetLayout(logicalDevice, cameraDescriptorSet.layout, nullptr);
	// Texture
	vkDestroyDescriptorSetLayout(logicalDevice, textureDescriptorSetLayout, nullptr);

	// Destroy pipeline and pipeline layout
	vkDestroyPipeline(logicalDevice, geometryPipeline, nullptr);
	vkDestroyPipelineLayout(logicalDevice, geometryPipelineLayout, nullptr);

	// Destroy default shader modules
	vkDestroyShaderModule(logicalDevice, geometryVertShader, nullptr);
	vkDestroyShaderModule(logicalDevice, geometryFragShader, nullptr);

	// Destroy all data from a material
	for (auto it = gameState->graphicsResources.materials.begin(); it != gameState->graphicsResources.materials.end(); it++)
	{
		Material* material = it->second;

		material->cleanUp(logicalDevice, vmaAllocator, descriptorPool);
		delete material;
		gameState->graphicsResources.materials.erase(it);
	}

	// Destroy all data from a mesh
	for (auto it = gameState->gameResources.entities.begin(); it != gameState->gameResources.entities.end(); it++)
	{
		Entity* entity = it->second;
		if (entity->HasComponent<MeshComponent>())
		{
			MeshComponent* meshComp = entity->GetComponent<MeshComponent>();
			gameState->graphicsResources.meshes[meshComp->meshIndex]->cleanup(logicalDevice, vmaAllocator);
			//	delete mesh;
		}
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
	vkFreeCommandBuffers(logicalDevice, commandPool, geometryBuffers.size(), geometryBuffers.data());
	vkDestroyCommandPool(logicalDevice, commandPool, nullptr);

	// Destroy framebuffer
	for (auto& framebuffer : framebuffers)
	{
		vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
	}

	// Destroy depth buffer
	vmaDestroyImage(vmaAllocator, depthImage.image, depthImage.allocation);
	vkDestroyImageView(logicalDevice, depthImage.imageView, nullptr);

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

	assert(imageIndex < geometryBuffers.size());
	assert(imageIndex < fences.size());

	// Make sure command buffer finishes executing
	vkResetCommandBuffer(preDepthBuffers[imageIndex], 0);
	vkResetCommandBuffer(shadowBuffers[imageIndex], 0);
	vkResetCommandBuffer(geometryBuffers[imageIndex], 0);
	vkResetCommandBuffer(shadingBuffers[imageIndex], 0);
	vkResetCommandBuffer(downscaleComputeCommandBuffers[imageIndex], 0);
	vkResetCommandBuffer(upscaleComputeCommandBuffers[imageIndex], 0);
	vkResetCommandBuffer(blendColorCommandBuffers[imageIndex], 0);
	vkResetCommandBuffer(presentCommandBuffers[imageIndex], 0);

	// Rendering command
	recordDepthPrePass(preDepthBuffers[imageIndex]);
	submitCommands(1, &preDepthBuffers[imageIndex], 1, &imageAvailable, 1, &preDepthFinished, graphicsQueue, nullptr);

	//if (gameState->graphicsResources.lights[1]->shouldRenderShadow())
	if (true)
	{
		recordShadowPass(shadowBuffers[imageIndex]);
		submitCommands(1, &shadowBuffers[imageIndex], 1, &preDepthFinished, 1, &shadowFinished, graphicsQueue, nullptr);
		gameState->graphicsResources.lights[1]->shadowRendered();

		recordGeometryPass(geometryBuffers[imageIndex]);
		submitCommands(1, &geometryBuffers[imageIndex], 1, &shadowFinished, 1, &geometryFinished, graphicsQueue, nullptr);
	}
	else
	{
		recordGeometryPass(geometryBuffers[imageIndex]);
		submitCommands(1, &geometryBuffers[imageIndex], 1, &preDepthFinished, 1, &geometryFinished, graphicsQueue, nullptr);
	}

	recordShadingPass(shadingBuffers[imageIndex]);
	submitCommands(1, &shadingBuffers[imageIndex], 1, &geometryFinished, 1, &renderFinished, graphicsQueue, nullptr);

	if (gameState->gameSettings.enableBloom)
	{
		// Record bloom commands if enabled
		recordDownscaleComputeCommands(downscaleComputeCommandBuffers[imageIndex]);
		submitCommands(1, &downscaleComputeCommandBuffers[imageIndex], 1, &renderFinished, 1, &downscaleFinished, graphicsQueue, nullptr);

		recordUpscaleComputeCommands(upscaleComputeCommandBuffers[imageIndex]);
		submitCommands(1, &upscaleComputeCommandBuffers[imageIndex], 1, &downscaleFinished, 1, &upscaleFinished, graphicsQueue, nullptr);

		recordBlendColorComputeCommands(blendColorCommandBuffers[imageIndex]);
		submitCommands(1, &blendColorCommandBuffers[imageIndex], 1, &upscaleFinished, 1, &colorBlendFinished, graphicsQueue, nullptr);

		recordUICommands(presentCommandBuffers[imageIndex], framebuffers[imageIndex], swapchainExtent);
		submitCommands(1, &presentCommandBuffers[imageIndex], 1, &colorBlendFinished, 1, &readyToPresent, graphicsQueue, &fences[imageIndex]);
	}
	else
	{
		recordUICommands(presentCommandBuffers[imageIndex], framebuffers[imageIndex], swapchainExtent);
		submitCommands(1, &presentCommandBuffers[imageIndex], 1, &renderFinished, 1, &readyToPresent, graphicsQueue, &fences[imageIndex]);
	}

	presentImage(graphicsQueue, readyToPresent, swapchain, imageIndex);

	// Update Texture after recording all rendering commands
	if (gameState->materialUpdateInfo.updateTexture || gameState->materialUpdateInfo.updateColor)
	{
		changeMaterialTexture(logicalDevice, physicalDevice, graphicsQueue, gameState);
		gameState->materialUpdateInfo.textureFilepath = "";
	}
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

void VulkanEngine::createShadingRenderPass(VkDevice& logicalDevice, VkRenderPass& renderPass, VkFormat format, const VkFormat& depthFormat)
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
			attachments[i].format = VK_FORMAT_R8G8B8A8_SRGB;

			if(i <= 1)
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
	WillEngine::VulkanUtil::createImageView(logicalDevice, depthImage.image, depthImage.imageView, 1, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VulkanEngine::createOffscreenAttachments(VkDevice& logicalDevice, const VkExtent2D& extent)
{
	// The back buffer
	// Create offscreen framebuffer attachments
	WillEngine::VulkanUtil::createFramebufferAttachment(logicalDevice, vmaAllocator, VK_FORMAT_R16G16B16A16_SFLOAT, extent, offscreenFramebuffer.GBuffer0);
	WillEngine::VulkanUtil::createFramebufferAttachment(logicalDevice, vmaAllocator, VK_FORMAT_R16G16B16A16_SFLOAT, extent, offscreenFramebuffer.GBuffer1);
	WillEngine::VulkanUtil::createFramebufferAttachment(logicalDevice, vmaAllocator, VK_FORMAT_R8G8B8A8_SRGB, extent, offscreenFramebuffer.GBuffer2);
	WillEngine::VulkanUtil::createFramebufferAttachment(logicalDevice, vmaAllocator, VK_FORMAT_R8G8B8A8_SRGB, extent, offscreenFramebuffer.GBuffer3);
	WillEngine::VulkanUtil::createFramebufferAttachment(logicalDevice, vmaAllocator, VK_FORMAT_R8G8B8A8_SRGB, extent, offscreenFramebuffer.GBuffer4);

	VkImageView attachments[6]{};
	attachments[0] = offscreenFramebuffer.GBuffer0.imageView;
	attachments[1] = offscreenFramebuffer.GBuffer1.imageView;
	attachments[2] = offscreenFramebuffer.GBuffer2.imageView;
	attachments[3] = offscreenFramebuffer.GBuffer3.imageView;
	attachments[4] = offscreenFramebuffer.GBuffer4.imageView;
	attachments[5] = depthImage.imageView;

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
		vkDestroyImageView(logicalDevice, depthImage.imageView, nullptr);
		vmaDestroyImage(vmaAllocator, depthImage.image, depthImage.allocation);

		// Create a new depth buffer
		//createDepthBuffer(logicalDevice, vmaAllocator, swapchainExtent);
		createDepthBuffer(logicalDevice, vmaAllocator, sceneExtent);

		// Shading
		vkDestroyImageView(logicalDevice, shadingImage.imageView, nullptr);
		vmaDestroyImage(vmaAllocator, shadingImage.image, shadingImage.allocation);

		// Computed
		for (u32 i = 0; i < downSampleImages.size(); i++)
		{
			vkDestroyImageView(logicalDevice, downSampleImages[i].imageView, nullptr);
			vmaDestroyImage(vmaAllocator, downSampleImages[i].image, downSampleImages[i].allocation);
		}
		for (u32 i = 0; i < upSampleImages.size(); i++)
		{
			vkDestroyImageView(logicalDevice, upSampleImages[i].imageView, nullptr);
			vmaDestroyImage(vmaAllocator, upSampleImages[i].image, upSampleImages[i].allocation);
		}

		vkFreeDescriptorSets(logicalDevice, descriptorPool, 1, &gameState->graphicsState.renderedImage.descriptorSet);
		vkDestroyDescriptorSetLayout(logicalDevice, gameState->graphicsState.renderedImage.layout, nullptr);
		vkFreeDescriptorSets(logicalDevice, vulkanGui->getDescriptorPool(), 1, &gameState->graphicsState.renderedImage_ImGui);


		//vkDestroyDescriptorSetLayout(logicalDevice, gameState->graphicsState.downSampledImageDescriptorSetLayout, nullptr);
		for (u32 i = 0; i < gameState->graphicsState.downSampledImageDescriptorSetOutput.size(); i++)
		{
			// Downscale
			// Output bindings
			vkFreeDescriptorSets(logicalDevice, descriptorPool, 1, &gameState->graphicsState.downSampledImageDescriptorSetOutput[i].descriptorSet);
			vkDestroyDescriptorSetLayout(logicalDevice, gameState->graphicsState.downSampledImageDescriptorSetOutput[i].layout, nullptr);
			// Input bindings
			vkFreeDescriptorSets(logicalDevice, descriptorPool, 1, &gameState->graphicsState.downSampledImageDescriptorSetInput[i].descriptorSet);
			vkDestroyDescriptorSetLayout(logicalDevice, gameState->graphicsState.downSampledImageDescriptorSetInput[i].layout, nullptr);

			// Upscale
			// Output bindings
			vkFreeDescriptorSets(logicalDevice, descriptorPool, 1, &gameState->graphicsState.upSampledImageDescriptorSetOutput[i].descriptorSet);
			vkDestroyDescriptorSetLayout(logicalDevice, gameState->graphicsState.upSampledImageDescriptorSetOutput[i].layout, nullptr);
			// Input bindings
			vkFreeDescriptorSets(logicalDevice, descriptorPool, 1, &gameState->graphicsState.upSampledImageDescriptorSetInput[i].descriptorSet);
			vkDestroyDescriptorSetLayout(logicalDevice, gameState->graphicsState.upSampledImageDescriptorSetInput[i].layout, nullptr);

			vkFreeDescriptorSets(logicalDevice, vulkanGui->getDescriptorPool(), 1, &gameState->graphicsState.downSampledImage_ImGui[i]);
			vkFreeDescriptorSets(logicalDevice, vulkanGui->getDescriptorPool(), 1, &gameState->graphicsState.upSampledImage_ImGui[i]);
		}
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

	WillEngine::VulkanUtil::createShadingImage(logicalDevice, vmaAllocator, swapchainImageFormat, sceneExtent, shadingImage);
	createShadingFramebuffer(logicalDevice, shadingFramebuffer, shadingRenderPass, sceneExtent);

	//WillEngine::VulkanUtil::createComputedImage(logicalDevice, vmaAllocator, commandPool, graphicsQueue, swapchainImageFormat, sceneExtent, postProcessedImage);

	VkExtent2D sceneExtentTemp = sceneExtent;

	for (u32 i = 0; i < downSampleImages.size(); i++)
	{
		WillEngine::VulkanUtil::createComputedImage(logicalDevice, vmaAllocator, commandPool, graphicsQueue, swapchainImageFormat, sceneExtentTemp, downSampleImages[i]);

		sceneExtentTemp.width /= 2;
		sceneExtentTemp.height /= 2;
	}

	sceneExtentTemp = sceneExtent;
	for (u32 i = 0; i < upSampleImages.size(); i++)
	{
		WillEngine::VulkanUtil::createComputedImage(logicalDevice, vmaAllocator, commandPool, graphicsQueue, swapchainImageFormat, sceneExtentTemp, upSampleImages[i]);

		sceneExtentTemp.width /= 2;
		sceneExtentTemp.height /= 2;
	}

	createSwapchainFramebuffer(logicalDevice, swapchainImageViews, framebuffers, offscreenFramebuffer, geometryRenderPass, shadingRenderPass, depthImage.imageView, swapchainExtent);

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
	attachments[0] = shadowCubeMap.imageView;

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
	attachments[0] = shadingImage.imageView;

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
	attachments[0] = depthImage.imageView;

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

void VulkanEngine::createCommandBuffers(VkDevice& logicalDevice)
{
	preDepthBuffers.resize(numSwapchainImage);
	shadowBuffers.resize(numSwapchainImage);
	geometryBuffers.resize(numSwapchainImage);
	shadingBuffers.resize(numSwapchainImage);
	downscaleComputeCommandBuffers.resize(numSwapchainImage);
	upscaleComputeCommandBuffers.resize(numSwapchainImage);
	blendColorCommandBuffers.resize(numSwapchainImage);
	presentCommandBuffers.resize(numSwapchainImage);

	for (u32 i = 0; i < numSwapchainImage; i++)
	{
		preDepthBuffers[i] = WillEngine::VulkanUtil::createCommandBuffer(logicalDevice, commandPool);
		shadowBuffers[i] = WillEngine::VulkanUtil::createCommandBuffer(logicalDevice, commandPool);
		geometryBuffers[i] = WillEngine::VulkanUtil::createCommandBuffer(logicalDevice, commandPool);
		shadingBuffers[i] = WillEngine::VulkanUtil::createCommandBuffer(logicalDevice, commandPool);
		downscaleComputeCommandBuffers[i] = WillEngine::VulkanUtil::createCommandBuffer(logicalDevice, commandPool);
		upscaleComputeCommandBuffers[i] = WillEngine::VulkanUtil::createCommandBuffer(logicalDevice, commandPool);
		blendColorCommandBuffers[i] = WillEngine::VulkanUtil::createCommandBuffer(logicalDevice, commandPool);
		presentCommandBuffers[i] = WillEngine::VulkanUtil::createCommandBuffer(logicalDevice, commandPool);
	}
}

void VulkanEngine::createSemaphore(VkDevice& logicalDevice)
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imageAvailable) != VK_SUCCESS)
		throw std::runtime_error("Failed to create wait image availabe semaphore");

	if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &renderFinished) != VK_SUCCESS)
		throw std::runtime_error("Failed to create signal render finish semaphore");

	if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &preDepthFinished) != VK_SUCCESS)
		throw std::runtime_error("Failed to create signal render finish semaphore");

	if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &shadowFinished) != VK_SUCCESS)
		throw std::runtime_error("Failed to create signal render finish semaphore");

	if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &geometryFinished) != VK_SUCCESS)
		throw std::runtime_error("Failed to create signal render finish semaphore");

	if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &downscaleFinished) != VK_SUCCESS)
		throw std::runtime_error("Failed to create signal ready to present semaphore");

	if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &upscaleFinished) != VK_SUCCESS)
		throw std::runtime_error("Failed to create signal ready to present semaphore");

	if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &colorBlendFinished) != VK_SUCCESS)
		throw std::runtime_error("Failed to create signal ready to present semaphore");


	if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &readyToPresent) != VK_SUCCESS)
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
	WillEngine::VulkanUtil::createDescriptorSetLayout(logicalDevice, shadowMapDescriptorSet.layout, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT, 3, 1);

	WillEngine::VulkanUtil::allocDescriptorSet(logicalDevice, descriptorPool, shadowMapDescriptorSet.layout, shadowMapDescriptorSet.descriptorSet);

	std::vector<VkImageView> imageViews = { shadowCubeMap.imageView };

	WillEngine::VulkanUtil::writeDescriptorSetImage(logicalDevice, shadowMapDescriptorSet.descriptorSet, &shadowSampler, imageViews.data(),
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

	WillEngine::VulkanUtil::createDescriptorSetLayout(logicalDevice, attachmentDescriptorSet.layout, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT, 2, 5);

	WillEngine::VulkanUtil::allocDescriptorSet(logicalDevice, descriptorPool, attachmentDescriptorSet.layout, attachmentDescriptorSet.descriptorSet);

	std::vector<VkImageView> imageViews = { offscreenFramebuffer.GBuffer0.imageView, offscreenFramebuffer.GBuffer1.imageView, offscreenFramebuffer.GBuffer2.imageView,
		offscreenFramebuffer.GBuffer3.imageView, offscreenFramebuffer.GBuffer4.imageView };

	WillEngine::VulkanUtil::writeDescriptorSetImage(logicalDevice, attachmentDescriptorSet.descriptorSet, &attachmentSampler, imageViews.data(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, imageViews.size());
}

void VulkanEngine::initRenderedDescriptors(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool)
{
	// Descriptor sets for ImGui
	gameState->graphicsState.renderedImage_ImGui = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(attachmentSampler, shadingImage.imageView, VK_IMAGE_LAYOUT_GENERAL);

	WillEngine::VulkanUtil::createDescriptorSetLayout(logicalDevice, gameState->graphicsState.renderedImage.layout, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		VK_SHADER_STAGE_COMPUTE_BIT, 0, 1);

	WillEngine::VulkanUtil::allocDescriptorSet(logicalDevice, descriptorPool, gameState->graphicsState.renderedImage.layout, gameState->graphicsState.renderedImage.descriptorSet);

	std::vector<VkImageView> imageViews = { shadingImage.imageView };

	WillEngine::VulkanUtil::writeDescriptorSetImage(logicalDevice, gameState->graphicsState.renderedImage.descriptorSet, &defaultSampler, imageViews.data(),
		VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, 1);
}

void VulkanEngine::initComputedImageDescriptors(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool)
{
	std::array<VulkanDescriptorSet, 6>& downSampledImageDescriptorSetInput = gameState->graphicsState.downSampledImageDescriptorSetInput;
	std::array<VulkanDescriptorSet, 6>& downSampledImageDescriptorSetOutput = gameState->graphicsState.downSampledImageDescriptorSetOutput;

	std::array<VulkanDescriptorSet, 7>& upSampledImageDescriptorSetInput = gameState->graphicsState.upSampledImageDescriptorSetInput;
	std::array<VulkanDescriptorSet, 7>& upSampledImageDescriptorSetOutput = gameState->graphicsState.upSampledImageDescriptorSetOutput;

	// Downscale
	// Output binding
	for (u32 i = 0; i < downSampledImageDescriptorSetOutput.size(); i++)
	{

		WillEngine::VulkanUtil::createDescriptorSetLayout(logicalDevice, downSampledImageDescriptorSetOutput[i].layout, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1, 1);

		WillEngine::VulkanUtil::allocDescriptorSet(logicalDevice, descriptorPool, downSampledImageDescriptorSetOutput[i].layout, downSampledImageDescriptorSetOutput[i].descriptorSet);

		std::vector<VkImageView> imageViews = { downSampleImages[i].imageView };

		WillEngine::VulkanUtil::writeDescriptorSetImage(logicalDevice, downSampledImageDescriptorSetOutput[i].descriptorSet, &defaultSampler, imageViews.data(), VK_IMAGE_LAYOUT_GENERAL,
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, 1);

		gameState->graphicsState.downSampledImage_ImGui[i] = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(defaultSampler, downSampleImages[i].imageView, VK_IMAGE_LAYOUT_GENERAL);
	}
	// Input binding
	for (u32 i = 0; i < downSampledImageDescriptorSetInput.size(); i++)
	{
		WillEngine::VulkanUtil::createDescriptorSetLayout(logicalDevice, downSampledImageDescriptorSetInput[i].layout, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 0, 1);

		WillEngine::VulkanUtil::allocDescriptorSet(logicalDevice, descriptorPool, downSampledImageDescriptorSetInput[i].layout, downSampledImageDescriptorSetInput[i].descriptorSet);

		std::vector<VkImageView> imageViews = { downSampleImages[i].imageView };

		WillEngine::VulkanUtil::writeDescriptorSetImage(logicalDevice, downSampledImageDescriptorSetInput[i].descriptorSet, &defaultSampler, imageViews.data(), VK_IMAGE_LAYOUT_GENERAL,
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, 1);
	}

	// Upscale
	// Output binding
	for (u32 i = 0; i < upSampledImageDescriptorSetOutput.size(); i++)
	{
		WillEngine::VulkanUtil::createDescriptorSetLayout(logicalDevice, upSampledImageDescriptorSetOutput[i].layout, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1, 1);
	
		WillEngine::VulkanUtil::allocDescriptorSet(logicalDevice, descriptorPool, upSampledImageDescriptorSetOutput[i].layout, upSampledImageDescriptorSetOutput[i].descriptorSet);
	
		std::vector<VkImageView> imageViews = { upSampleImages[i].imageView };

		WillEngine::VulkanUtil::writeDescriptorSetImage(logicalDevice, upSampledImageDescriptorSetOutput[i].descriptorSet, &defaultSampler, imageViews.data(), VK_IMAGE_LAYOUT_GENERAL,
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, 1);

		gameState->graphicsState.upSampledImage_ImGui[i] = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(defaultSampler, upSampleImages[i].imageView, VK_IMAGE_LAYOUT_GENERAL);
	}
	// Input binding
	for (u32 i = 0; i < upSampledImageDescriptorSetInput.size(); i++)
	{
		WillEngine::VulkanUtil::createDescriptorSetLayout(logicalDevice, upSampledImageDescriptorSetInput[i].layout, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 0, 1);

		WillEngine::VulkanUtil::allocDescriptorSet(logicalDevice, descriptorPool, upSampledImageDescriptorSetInput[i].layout, upSampledImageDescriptorSetInput[i].descriptorSet);

		std::vector<VkImageView> imageViews = { upSampleImages[i].imageView };

		WillEngine::VulkanUtil::writeDescriptorSetImage(logicalDevice, upSampledImageDescriptorSetInput[i].descriptorSet, &defaultSampler, imageViews.data(), VK_IMAGE_LAYOUT_GENERAL,
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, 1);
	}
}

void VulkanEngine::initGeometryPipeline(VkDevice& logicalDevice)
{
	// Set up shader modules
	WillEngine::VulkanUtil::initGeometryShaderModule(logicalDevice, geometryVertShader, geometryFragShader);

	// Create pipeline and pipeline layout
	VkDescriptorSetLayout layouts[] = { sceneDescriptorSet.layout, textureDescriptorSetLayout };
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
	VkDescriptorSetLayout depthLayouts[] = { sceneDescriptorSet.layout };
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

	WillEngine::VulkanUtil::createDepthImageView(logicalDevice, shadowCubeMap.image, shadowCubeMap.imageView, 6, shadowDepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	WillEngine::VulkanUtil::createDepthSampler(logicalDevice, shadowSampler);

	// Shadow Framebuffer
	createShadowFramebuffer(logicalDevice, shadowFramebuffer, shadowRenderPass, 1024, 1024);

	VkDescriptorSetLayout layout[] = { lightMatrixDescriptorSet.layout, lightDescriptorSet.layout };
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

	VkDescriptorSetLayout layout[] = { lightDescriptorSet.layout, cameraDescriptorSet.layout, attachmentDescriptorSet.layout, shadowMapDescriptorSet.layout };
	u32 layoutSize = sizeof(layout) / sizeof(layout[0]);

	WillEngine::VulkanUtil::createPipelineLayout(logicalDevice, shadingPipelineLayout, layoutSize, layout, 0, nullptr);

	WillEngine::VulkanUtil::createShadingPipeline(logicalDevice, shadingPipeline, shadingPipelineLayout, shadingRenderPass, shadingVertShader, shadingFragShader,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, sceneExtent);
}

void VulkanEngine::initFilterBrightPipeline(VkDevice& logicalDevice)
{
	WillEngine::VulkanUtil::initFilterBrightShaderModule(logicalDevice, filterBrightCompShader);
	
	VkDescriptorSetLayout layout[] = { gameState->graphicsState.renderedImage.layout, gameState->graphicsState.downSampledImageDescriptorSetOutput[0].layout };
	u32 layoutSize = sizeof(layout) / sizeof(layout[0]);

	WillEngine::VulkanUtil::createPipelineLayout(logicalDevice, filterBrightPipelineLayout, layoutSize, layout, 0, nullptr);

	WillEngine::VulkanUtil::createComputePipeline(logicalDevice, filterBrightPipeline, filterBrightPipelineLayout, filterBrightCompShader);
}

void VulkanEngine::initDownscalePipeline(VkDevice& logicalDevice)
{
	WillEngine::VulkanUtil::initDownscaleShaderModule(logicalDevice, downscaleCompShader);

	VkDescriptorSetLayout layout[] = { gameState->graphicsState.downSampledImageDescriptorSetInput[0].layout, gameState->graphicsState.downSampledImageDescriptorSetOutput[1].layout };
	u32 layoutSize = sizeof(layout) / sizeof(layout[0]);

	WillEngine::VulkanUtil::createPipelineLayout(logicalDevice, downscalePipelineLayout, layoutSize, layout, 0, nullptr);

	WillEngine::VulkanUtil::createComputePipeline(logicalDevice, downscalePipeline, downscalePipelineLayout, downscaleCompShader);
}

void VulkanEngine::initUpscalePipeline(VkDevice& logicalDevice)
{
	WillEngine::VulkanUtil::initUpscaleShaderModule(logicalDevice, upscaleCompShader);

	VkDescriptorSetLayout layout[]{ gameState->graphicsState.upSampledImageDescriptorSetInput[0].layout, gameState->graphicsState.upSampledImageDescriptorSetOutput[0].layout };
	u32 layoutSize = sizeof(layout) / sizeof(layout[0]);

	WillEngine::VulkanUtil::createPipelineLayout(logicalDevice, upscalePipelineLayout, layoutSize, layout, 0, nullptr);
	WillEngine::VulkanUtil::createComputePipeline(logicalDevice, upscalePipeline, upscalePipelineLayout, upscaleCompShader);
}

void VulkanEngine::initBlendColorPipeline(VkDevice& logicalDevice)
{
	WillEngine::VulkanUtil::initBlendColorShaderModule(logicalDevice, blendColorCompShader);

	VkDescriptorSetLayout layout[]{ gameState->graphicsState.renderedImage.layout, gameState->graphicsState.upSampledImageDescriptorSetOutput[0].layout };
	u32 layoutSize = sizeof(layout) / sizeof(layout[0]);

	WillEngine::VulkanUtil::createPipelineLayout(logicalDevice, blendColorPipelineLayout, layoutSize, layout, 0, nullptr);
	WillEngine::VulkanUtil::createComputePipeline(logicalDevice, blendColorPipeline, blendColorPipelineLayout, blendColorCompShader);
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
	vkCmdUpdateBuffer(commandBuffer, lightUniformBuffer.buffer, 0, sizeof(gameState->graphicsResources.lights[1]->lightUniform), &gameState->graphicsResources.lights[1]->lightUniform);

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
	//if (gameState->graphicsResources.lights[0]->shouldRenderShadow())
	if (true)
	{
		shadowPasses(commandBuffer);
		gameState->graphicsResources.lights[1]->shadowRendered();
	}

	// Shading
	shadingPasses(commandBuffer, shadingRenderPass, shadingFramebuffer, sceneExtent);

	// ========================================================

	// End command buffer
	vkEndCommandBuffer(commandBuffer);
}

void VulkanEngine::recordDepthPrePass(VkCommandBuffer& commandBuffer)
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
	vkCmdUpdateBuffer(commandBuffer, lightUniformBuffer.buffer, 0, sizeof(gameState->graphicsResources.lights[1]->lightUniform), &gameState->graphicsResources.lights[1]->lightUniform);

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

	depthPrePasses(commandBuffer, sceneExtent);

	// End command buffer
	vkEndCommandBuffer(commandBuffer);
}

void VulkanEngine::recordShadowPass(VkCommandBuffer& commandBuffer)
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
	vkCmdUpdateBuffer(commandBuffer, lightUniformBuffer.buffer, 0, sizeof(gameState->graphicsResources.lights[1]->lightUniform), &gameState->graphicsResources.lights[1]->lightUniform);

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

	shadowPasses(commandBuffer);
	gameState->graphicsResources.lights[1]->shadowRendered();

	// End command buffer
	vkEndCommandBuffer(commandBuffer);
}

void VulkanEngine::recordGeometryPass(VkCommandBuffer& commandBuffer)
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
	vkCmdUpdateBuffer(commandBuffer, lightUniformBuffer.buffer, 0, sizeof(gameState->graphicsResources.lights[1]->lightUniform), &gameState->graphicsResources.lights[1]->lightUniform);

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

	// Deferred rendering pass
	geometryPasses(commandBuffer, sceneExtent);

	// End command buffer
	vkEndCommandBuffer(commandBuffer);
}

void VulkanEngine::recordShadingPass(VkCommandBuffer& commandBuffer)
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
	vkCmdUpdateBuffer(commandBuffer, lightUniformBuffer.buffer, 0, sizeof(gameState->graphicsResources.lights[1]->lightUniform), &gameState->graphicsResources.lights[1]->lightUniform);

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

	// Shading
	shadingPasses(commandBuffer, shadingRenderPass, shadingFramebuffer, sceneExtent);

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
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, depthPipelineLayout, 0, 1, &sceneDescriptorSet.descriptorSet, 0, nullptr);

	for (auto it = gameState->gameResources.entities.begin(); it != gameState->gameResources.entities.end(); it++)
	{
		Entity* entity = it->second;

		MeshComponent* meshComponent = entity->GetComponent<MeshComponent>();

		if (!entity->isEnable)
			continue;

		if (!meshComponent)
			continue;

		if (!gameState->graphicsResources.meshes[meshComponent->meshIndex]->isReadyToDraw())
			continue;

		TransformComponent* transformComponent = entity->GetComponent<TransformComponent>();
		Mesh* mesh = gameState->graphicsResources.meshes[meshComponent->meshIndex];

		VkBuffer buffers[3] = { mesh->positionBuffer.buffer, mesh->normalBuffer.buffer, mesh->uvBuffer.buffer };

		VkDeviceSize offsets[3]{};

		// Bind buffers
		vkCmdBindVertexBuffers(commandBuffer, 0, 3, buffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer, mesh->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		// Bind Texture
		// Check if this mesh has a material first
		if(gameState->graphicsResources.materials[meshComponent->materialIndex])
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, geometryPipelineLayout, 1, 1, &gameState->graphicsResources.materials[meshComponent->materialIndex]->textureDescriptorSet, 0, nullptr);

		// Push constant for model matrix
		vkCmdPushConstants(commandBuffer, geometryPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
			sizeof(transformComponent->getModelTransformation()), &transformComponent->getModelTransformation());

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
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, geometryPipelineLayout, 0, 1, &sceneDescriptorSet.descriptorSet, 0, nullptr);

	for(auto it = gameState->gameResources.entities.begin(); it != gameState->gameResources.entities.end(); it++)
	{
		Entity* entity = it->second;

		MeshComponent* meshComponent = entity->GetComponent<MeshComponent>();

		if (!entity->isEnable)
			continue;

		if (!meshComponent)
			continue;

		if (!gameState->graphicsResources.meshes[meshComponent->meshIndex]->isReadyToDraw())
			continue;

		TransformComponent* transformComponent = entity->components[typeid(TransformComponent)]->GetComponent<TransformComponent>();
		Mesh* mesh = gameState->graphicsResources.meshes[meshComponent->meshIndex];

		VkBuffer buffers[3] = { mesh->positionBuffer.buffer, mesh->normalBuffer.buffer, mesh->uvBuffer.buffer };

		VkDeviceSize offsets[3]{};

		// Bind buffers
		vkCmdBindVertexBuffers(commandBuffer, 0, 3, buffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer, mesh->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		// Bind Texture
		// Check if the mesh has a material
		if(gameState->graphicsResources.materials[meshComponent->materialIndex])
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, geometryPipelineLayout, 1, 1, &gameState->graphicsResources.materials[meshComponent->materialIndex]->textureDescriptorSet, 0, nullptr);

		// Push constant for model matrix
		vkCmdPushConstants(commandBuffer, geometryPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, 
			sizeof(transformComponent->getModelTransformation()), &transformComponent->getModelTransformation());

		vkCmdDrawIndexed(commandBuffer, static_cast<u32>(mesh->indiciesSize), 3, 0, 0, 0);
	}

	// End Render Pass
	vkCmdEndRenderPass(commandBuffer);
}

void VulkanEngine::shadowPasses(VkCommandBuffer& commandBuffer)
{
	// Update light matrices buffer
	vkCmdUpdateBuffer(commandBuffer, lightMatrixUniformBuffer.buffer, 0, sizeof(mat4) * 6, &gameState->graphicsResources.lights[1]->matrices);

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
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipelineLayout, 1, 1, &lightDescriptorSet.descriptorSet, 0, nullptr);

	// Bind light matrices
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipelineLayout, 0, 1, &lightMatrixDescriptorSet.descriptorSet, 0, nullptr);

	for(auto it = gameState->gameResources.entities.begin(); it != gameState->gameResources.entities.end(); it++)
	{
		Entity* entity = it->second;

		MeshComponent* meshComponent = entity->GetComponent<MeshComponent>();

		if (!entity->isEnable)
			continue;

		if (!meshComponent)
			continue;

		if (!gameState->graphicsResources.meshes[meshComponent->meshIndex]->isReadyToDraw())
			continue;

		// Ignore this entity if it is a light
		if (entity->HasComponent<LightComponent>())
			continue;

		Mesh* mesh = gameState->graphicsResources.meshes[meshComponent->meshIndex];

		VkBuffer buffers[3] = { mesh->positionBuffer.buffer, mesh->normalBuffer.buffer, mesh->uvBuffer.buffer };

		VkDeviceSize offsets[3]{};

		// Bind buffers
		vkCmdBindVertexBuffers(commandBuffer, 0, 3, buffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer, mesh->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		// Push constant for model matrix
		TransformComponent* transform = entity->GetComponent<TransformComponent>();
		vkCmdPushConstants(commandBuffer, geometryPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(transform->getModelTransformation()), &transform->getModelTransformation());

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

	// Bind Light Uniform Buffer
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadingPipelineLayout, 0, 1, &lightDescriptorSet.descriptorSet, 0, nullptr);

	// Bind Camera Uniform Buffer
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadingPipelineLayout, 1, 1, &cameraDescriptorSet.descriptorSet, 0, nullptr);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadingPipelineLayout, 2, 1, &attachmentDescriptorSet.descriptorSet, 0, nullptr);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadingPipelineLayout, 3, 1, &shadowMapDescriptorSet.descriptorSet, 0, nullptr);

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

void VulkanEngine::recordDownscaleComputeCommands(VkCommandBuffer& commandBuffer)
{
	// Begin command buffer
	VkCommandBufferBeginInfo commandBeginInfo{};
	commandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(commandBuffer, &commandBeginInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to begin command buffer");

	// Bind Pipeline for filtering bright color
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, filterBrightPipeline);

	// Bind Descriptor sets
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, filterBrightPipelineLayout, 0, 1, &gameState->graphicsState.renderedImage.descriptorSet, 0, nullptr);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, filterBrightPipelineLayout, 1, 1,
		&gameState->graphicsState.downSampledImageDescriptorSetOutput[0].descriptorSet, 0, nullptr);

	// Dispatch compute job.
	vkCmdDispatch(commandBuffer, sceneExtent.width / 16, sceneExtent.height / 16, 1);

	// Bind Pipeline for downscaling
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, downscalePipeline);

	// Downscaling from mip 0 to mip 5
	for (u32 i = 1; i < gameState->graphicsState.downSampledImageDescriptorSetOutput.size(); i++)
	{
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, downscalePipelineLayout, 0, 1,
			&gameState->graphicsState.downSampledImageDescriptorSetInput[i-1].descriptorSet, 0, nullptr);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, downscalePipelineLayout, 1, 1,
			&gameState->graphicsState.downSampledImageDescriptorSetOutput[i].descriptorSet, 0, nullptr);

		vkCmdDispatch(commandBuffer, sceneExtent.width / 16, sceneExtent.height / 16, 1);
	}

	// We downscale the last image to the last mip level of upSampled instead of downSampled
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, downscalePipelineLayout, 0, 1,
		&gameState->graphicsState.downSampledImageDescriptorSetInput[gameState->graphicsState.downSampledImageDescriptorSetOutput.size() - 1].descriptorSet, 0, nullptr);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, downscalePipelineLayout, 1, 1,
		&gameState->graphicsState.upSampledImageDescriptorSetOutput[gameState->graphicsState.upSampledImageDescriptorSetOutput.size() - 1].descriptorSet, 0, nullptr);

	vkCmdDispatch(commandBuffer, sceneExtent.width / 16, sceneExtent.height / 16, 1);

	vkEndCommandBuffer(commandBuffer);
}

void VulkanEngine::recordUpscaleComputeCommands(VkCommandBuffer& commandBuffer)
{
	// Begin command buffer
	VkCommandBufferBeginInfo commandBeginInfo{};
	commandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(commandBuffer, &commandBeginInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to begin command buffer");

	// Upscaling the image
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, upscalePipeline);

	// Upscaling from mip 6 to mip 0
	for (u32 i = gameState->graphicsState.upSampledImageDescriptorSetInput.size() - 1; i > 0; i--)
	{
		// Result Image
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, upscalePipelineLayout, 0, 1,
			&gameState->graphicsState.upSampledImageDescriptorSetInput[i-1].descriptorSet, 0, nullptr);

		// Input Image
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, upscalePipelineLayout, 1, 1,
			&gameState->graphicsState.upSampledImageDescriptorSetOutput[i].descriptorSet, 0, nullptr);

		vkCmdDispatch(commandBuffer, sceneExtent.width / 16, sceneExtent.height / 16, 1);
	}

	vkEndCommandBuffer(commandBuffer);
}

void VulkanEngine::recordBlendColorComputeCommands(VkCommandBuffer& commandBuffer)
{
	// Begin command buffer
	VkCommandBufferBeginInfo commandBeginInfo{};
	commandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(commandBuffer, &commandBeginInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to begin command buffer");

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, blendColorPipeline);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, blendColorPipelineLayout, 0, 1,
		&gameState->graphicsState.renderedImage.descriptorSet, 0, nullptr);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, blendColorPipelineLayout, 1, 1,
		&gameState->graphicsState.upSampledImageDescriptorSetOutput[0].descriptorSet, 0, nullptr);

	vkCmdDispatch(commandBuffer, sceneExtent.width / 16, sceneExtent.height / 16, 1);

	vkEndCommandBuffer(commandBuffer);
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

void VulkanEngine::submitCommands(u32 commandBufferCount, VkCommandBuffer* commandBuffer, u32 waitSemaphoreCount, VkSemaphore* waitSemaphore, u32 signalSemaphoreCount, 
	VkSemaphore* signalSemaphore, VkQueue& graphicsQueue, VkFence* fence)
{
	VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = waitSemaphoreCount;
	submitInfo.pWaitSemaphores = waitSemaphore;
	submitInfo.pWaitDstStageMask = &dstStageMask;
	submitInfo.commandBufferCount = commandBufferCount;
	submitInfo.pCommandBuffers = commandBuffer;
	submitInfo.signalSemaphoreCount = signalSemaphoreCount;
	submitInfo.pSignalSemaphores = signalSemaphore;

	if (fence == nullptr)
		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	else
		vkQueueSubmit(graphicsQueue, 1, &submitInfo, *fence);
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

	Material* currentMaterial = gameState->graphicsResources.materials[gameState->materialUpdateInfo.materialId];
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