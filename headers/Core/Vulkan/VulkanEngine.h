#pragma once
#include "Core/Camera.h"
#include "Core/MeshComponent.h"
#include "Core/ECS/SkinnedMeshComponent.h"
#include "Core/ECS/SkeletalComponent.h"
#include "Core/Material.h"
#include "Core/SkinnedMesh.h"
#include "Core/LightComponent.h"
#include "Core/Camera.h"
#include "Core/UniformClass.h"

#include "Core/Vulkan/VulkanDefines.h"
#include "Core/Vulkan/VulkanGui.h"

#include "Core/ECS/TransformComponent.h"

#include "Utils/VulkanUtil.h"

#include "Core/GameState.h"

class VulkanEngine
{
private:

	// Max number of threads this vulkan renderer can use
	const u32 MAX_THREADS;

public:

	GameState* gameState;

	Camera* camera;


public:

	// Triple buffering
	static const u32 NUM_SWAPCHAIN = 3;

	const VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

	const VkFormat shadowDepthFormat = VK_FORMAT_D32_SFLOAT;

	const u32 numDownSampleImage = 6;
	const u32 numUpSampleImage = 7;

public:

	std::queue<Skeleton*> skeletonToInitialise;

public:

	VmaAllocator vmaAllocator;

	std::unordered_map<VulkanRenderPassType, VkRenderPass> renderPasses;

	// Swapchain
	VkSwapchainKHR swapchain;

	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;

	VkFormat swapchainImageFormat;

	// The whole window application size
	VkExtent2D swapchainExtent;

	// The scene window size
	VkExtent2D sceneExtent = {100, 100};
	bool sceneExtentChanged = true;

	std::unordered_map<VulkanFramebufferType, VulkanAllocatedImage> framebuffersImages;
	std::unordered_map<VulkanFramebufferType, VulkanFramebuffer> framebuffers;

	std::unordered_map<VulkanPostProcessingType, VulkanPostProcessingImages> postProcessingImages;

	std::vector<VkFramebuffer> presentFramebuffers;

	// Sampler to sample framebuffer's color attachment
	std::unordered_map<VulkanSamplerType, VkSampler> samplers;

	// Command pool and buffer
	// The first command pools are used for allocation/initialisation, the rests is used for recording command buffers for rendering commands
	VkCommandPool commandPool;

	// Secondary Command buffers that store the rendering command that will be used several times in a rendering loop
	std::vector<VkCommandBuffer> depthMeshBuffers;
	std::vector<VkCommandBuffer> depthSkeletalBuffers;

	std::vector<VkCommandBuffer> shadowMeshBuffers;
	std::vector<VkCommandBuffer> shadowSkeletalBuffers;

	std::vector<VkCommandBuffer> geometryMeshBuffers;
	std::vector<VkCommandBuffer> geometrySkeletalBuffers;


	// Primary Command Buffers
	std::unordered_map<VulkanCommandBufferType, std::vector<VkCommandBuffer>> pipelineCommandBuffers;

	// Semaphore for waiting and signaling
	// Used for GPU - GPU sync
	std::unordered_map<VulkanSemaphoreType, VkSemaphore> semaphores;

	// Fence 
	// Used for CPU - GPU symc
	std::vector<VkFence> fences;

	VkDescriptorPool descriptorPool;

	// Pipeline and pipeline layout (Blinn Phong Shader)
	std::vector<VulkanPipeline> pipelines;
	std::unordered_map<VulkanPipelineType, u32> pipelineIndexLookup;

	std::unordered_map<VulkanDescriptorSetType, VulkanDescriptorSet> descriptorSets;

	// Shader modules
	std::unordered_map<VulkanPipelineType, VulkanShaderModule> pipelineShaders;

	// GUI
	VulkanGui* vulkanGui;

private:

	CameraMatrix sceneMatrix;

public:

	VulkanEngine(u32 numThreads);
	~VulkanEngine();

	void init(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR surface, VkQueue& graphicsQueue, 
		GameState* gameState);
	void cleanup(VkDevice& logicalDevice);

	// Init / setup

	void createVmaAllocator(VkInstance& instance, VkPhysicalDevice& physicalDevice, VkDevice& logicalDevice);

	void createSwapchain(GLFWwindow* window, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);
	void getSwapchainImages(VkDevice& logicalDevice);
	void createSwapchainImageViews(VkDevice& logicalDevice);

	void createDepthPrePass(VkDevice& logicalDevice, VkRenderPass& renderPass, const VkFormat& depthFormat);
	void createPresentRenderPass(VkDevice& logicalDevice, VkRenderPass& renderPass, const VkFormat& format);
	void createShadingRenderPass(VkDevice& logicalDevice, VkRenderPass& renderPass, VkFormat format, const VkFormat& depthFormat);
	void createShadowRenderPass(VkDevice& logicalDevice, VkRenderPass& renderPass, const VkFormat& depthFormat);
	void createGeometryRenderPass(VkDevice& logicalDevice, VkRenderPass& renderPass, VkFormat format, const VkFormat& depthFormat);

	void createDepthBuffer(VkDevice& logicalDevice, VmaAllocator& vmaAllocator, const VkExtent2D& extent);
	void destroyDepthBuffer(VkDevice& logicalDevice, VmaAllocator& vmaAllocator);

	void createGBuffers(VkDevice& logicalDevice, const VkExtent2D& extent);

	void createSwapchainFramebuffer(VkDevice& logicalDevice, std::vector<VkImageView>& swapchainImageViews,
		std::vector<VkFramebuffer>& framebuffers, VulkanFramebuffer& offscreenFramebuffer, VkRenderPass& geometryRenderPass, VkRenderPass& renderPass, VkImageView& depthImageView, 
		VkExtent2D extent);
	void recreateSwapchainFramebuffer(GLFWwindow* window, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkQueue graphicsQueue);

	void createShadowFramebuffer(VkDevice& logicalDevice, VkFramebuffer& shadowFramebuffer, VkRenderPass& shadowRenderPass, u32 width, u32 height);

	void createShadingFramebuffer(VkDevice& logicalDevice, VkFramebuffer& shadingFramebuffer, VkRenderPass& shadingRenderPass, VkExtent2D extent);

	void createDepthFramebuffer(VkDevice& logicalDevice, VkFramebuffer& depthFramebuffer, VkRenderPass& depthRenderPass, VkExtent2D extent);

	void createImageBuffersForPostProcessing(VkDevice& logicalDevice, VkQueue& graphicsQueue);
	void destroyImageBuffersForPostProcessing(VkDevice& logicalDevice, VmaAllocator& vmaAllocator);

	void createCommandPool(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkCommandPool& commandPool);

	void createCommandBuffers(VkDevice& logicalDevice);
	void createSecondaryCommandBuffers(VkDevice& logicalDevice);

	void createSemaphore(VkDevice& logicalDevice);

	void createFence(VkDevice& logicalDevice, std::vector<VkFence>& fences, VkFenceCreateFlagBits flag);

	void createDescriptionPool(VkDevice& logicalDevice);

	void initUniformBuffer(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool, VulkanAllocatedMemory& uniformBuffer,VkDescriptorSet& descriptorSet, 
		VkDescriptorSetLayout& descriptorSetLayout, u32 binding, u32 bufferSize, VkShaderStageFlagBits shaderStage);
	void createUniformBuffer(VkDevice& logicalDevice, VulkanAllocatedMemory& uniformBuffer, u32 bufferSize);

	void initDescriptorSets(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool);

	// Initialise descriptor sets for shadow mapping
	void initShadowMapDescriptors(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool, VulkanDescriptorSet& descriptorSet);

	// Initialise descriptor sets for deferred rendering
	void initAttachmentDescriptors(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool, VulkanDescriptorSet& descriptorSet);

	void initRenderedDescriptors(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool);

	void initComputedImageDescriptors(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool);
	void freeComputedImageDescriptors(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool);

	// Pipeline init
	void initDepthSkeletalPipeline(VkDevice& logicalDevice);
	void initSkeletalPipeline(VkDevice& logicalDevice);
	void initGeometryPipeline(VkDevice& logicalDevice);
	void initDepthPipeline(VkDevice& logicalDevice);
	void initShadowPipeline(VkDevice& logicalDevice);
	void initShadingPipeline(VkDevice& logicalDevice);

	void initFilterBrightPipeline(VkDevice& logicalDevice);
	void initDownscalePipeline(VkDevice& logicalDevice);
	void initUpscalePipeline(VkDevice& logicalDevice);
	void initBlendColorPipeline(VkDevice& logicalDevice);

	// GUI
	void initGui(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkQueue& queue, VkSurfaceKHR& surface);

	// Update
	void update(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR surface, VkQueue graphicsQueue, bool renderWithBRDF);

	void updateSceneUniform(Camera* camera);
	void updateSkeletonUniform(VkCommandBuffer& commandBuffer);

	void processTodoSkeleton(VkDevice& logicalDevice);

	void recordUniformUpdate(VkCommandBuffer& commandBuffer);

	// Record rendering commands
	void recordMeshSecondaryCommandBuffer(VkCommandBuffer& commandBuffer, VkRenderPass& renderPass, VkFramebuffer& framebuffer, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout);
	void recordSkeletalSecondaryCommandBuffer(VkCommandBuffer& commandBuffer, VkRenderPass& renderPass, VkFramebuffer& framebuffer, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout);

	void recordDepthPrePass(VkCommandBuffer& commandBuffer, VkCommandBuffer& meshBuffer, VkCommandBuffer& skeletalBuffer);
	void recordShadowPass(VkCommandBuffer& commandBuffer);
	void recordGeometryPass(VkCommandBuffer& commandBuffer, VkCommandBuffer& meshBuffer, VkCommandBuffer& skeletalBuffer);
	void recordShadingPass(VkCommandBuffer& commandBuffer);

	// The actual render passes commands
	void depthSkeletalPrePasses(VkCommandBuffer& commandBuffer);
	void depthPrePasses(VkCommandBuffer& commandBuffer);
	void geometrySkeletalPasses(VkCommandBuffer& commandBuffer, VkExtent2D extent);
	void geometryPasses(VkCommandBuffer& commandBuffer, VkExtent2D extent);
	void shadowPasses(VkCommandBuffer& commandBuffer);
	void shadingPasses(VkCommandBuffer& commandBuffer, VkRenderPass& renderPass, VkFramebuffer& framebuffer, VkExtent2D extent);
	void UIPasses(VkCommandBuffer& commandBuffer, VkRenderPass& renderPass, VkFramebuffer& framebuffer, VkExtent2D extent);

	// Post-processing
	void recordDownscaleComputeCommands(VkCommandBuffer& commandBuffer);
	void recordUpscaleComputeCommands(VkCommandBuffer& commandBuffer);
	void recordBlendColorComputeCommands(VkCommandBuffer& commandBuffer);

	void recordUICommands(VkCommandBuffer& commandBuffer, VkFramebuffer& framebuffer, VkExtent2D& extent);

	void submitCommands(u32 commandBufferCount, VkCommandBuffer* commandBuffer, u32 waitSemaphoreCount, VkSemaphore* waitSemaphore, u32 signalSemaphoreCount,
		VkSemaphore* signalSemaphore, VkQueue& graphicsQueue, VkFence* fence);

	void presentImage(VkQueue& graphicsQueue, VkSemaphore& waitSemaphore, VkSwapchainKHR& swapchain, u32& swapchainIndex);


private:
	VkSurfaceFormatKHR selectSwapchainSurfaceFormat(std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats);

	VkPresentModeKHR selectSwapchainPresentMode(std::vector<VkPresentModeKHR>& presentModes);

	void changeMaterialTexture(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue, GameState* gameState);
};