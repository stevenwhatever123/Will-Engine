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

public:

	std::queue<Skeleton*> skeletonToInitialise;

public:

	VmaAllocator vmaAllocator;

	VkRenderPass depthRenderPass;
	VkRenderPass geometryRenderPass;
	VkRenderPass shadowRenderPass;
	VkRenderPass shadingRenderPass;
	VkRenderPass presentRenderPass;

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

	// Depth buffer
	VulkanAllocatedImage depthImage;

	// Shadow map framebuffer
	VkFramebuffer shadowFramebuffer;

	// Framebuffer for depth pre-pass
	VkFramebuffer depthFramebuffer;

	VulkanAllocatedImage shadingImage;
	VkFramebuffer shadingFramebuffer;

	//VulkanAllocatedImage postProcessedImage;

	std::array<VulkanAllocatedImage, 6> downSampleImages;
	std::array<VulkanAllocatedImage, 7> upSampleImages;

	// Framebuffer
	std::vector<VkFramebuffer> framebuffers;
	// Stored as a vector as there are multiple framebuffers in a swapchain
	VulkanFramebuffer offscreenFramebuffer;

	// Sampler to sample framebuffer's color attachment
	VkSampler attachmentSampler;
	VkSampler defaultSampler;

	// Command pool and buffer
	// The first command pools are used for allocation/initialisation, the rests is used for recording command buffers for rendering commands
	std::vector<VkCommandPool> commandPools;
	std::vector<VkCommandBuffer> uniformUpdateBuffers;

	// Secondary Command buffers that store the rendering command that will be used several times in a rendering loop
	std::vector<VkCommandBuffer> depthMeshBuffers;
	std::vector<VkCommandBuffer> depthSkeletalBuffers;

	std::vector<VkCommandBuffer> shadowMeshBuffers;
	std::vector<VkCommandBuffer> shadowSkeletalBuffers;

	std::vector<VkCommandBuffer> geometryMeshBuffers;
	std::vector<VkCommandBuffer> geometrySkeletalBuffers;


	// Primary Command Buffers
	std::vector<VkCommandBuffer> preDepthBuffers;
	std::vector<VkCommandBuffer> shadowBuffers;
	std::vector<VkCommandBuffer> geometryBuffers;
	std::vector<VkCommandBuffer> shadingBuffers;
	std::vector<VkCommandBuffer> downscaleComputeCommandBuffers;
	std::vector<VkCommandBuffer> upscaleComputeCommandBuffers;
	std::vector<VkCommandBuffer> blendColorCommandBuffers;
	std::vector<VkCommandBuffer> presentCommandBuffers;

	// Semaphore for waiting and signaling
	// Used for GPU - GPU sync
	VkSemaphore imageAvailable;
	VkSemaphore uniformUpdated;
	VkSemaphore renderFinished;

	VkSemaphore preDepthFinished;
	VkSemaphore shadowFinished;
	VkSemaphore geometryFinished;

	VkSemaphore downscaleFinished;
	VkSemaphore upscaleFinished;

	VkSemaphore colorBlendFinished;

	VkSemaphore readyToPresent;

	// Fence 
	// Used for CPU - GPU symc
	std::vector<VkFence> fences;

	VkDescriptorPool descriptorPool;

	// Pipeline and pipeline layout (Blinn Phong Shader)
	std::vector<VulkanPipeline> pipelines;
	std::unordered_map<String, u32> pipelineIndexLookup;

	// Bone Descriptor sets
	VkDescriptorSetLayout depthSkeletalDescriptorSetLayout;
	VkDescriptorSetLayout skeletalDescriptorSetLayout;

	// Scene Descriptor sets
	VulkanDescriptorSet sceneDescriptorSet;
	// Scene Uniform buffer
	VulkanAllocatedMemory sceneUniformBuffer;

	// Light Descriptor sets
	VulkanDescriptorSet lightDescriptorSet;
	// Lights Uniform buffer
	VulkanAllocatedMemory lightUniformBuffer;

	// Camera Descriptor sets
	VulkanDescriptorSet cameraDescriptorSet;
	// Camera Uniform buffer
	VulkanAllocatedMemory cameraUniformBuffer;

	// Texture Descriptor sets
	VkDescriptorSetLayout textureDescriptorSetLayout;

	// Descriptor sets for shading from deferred rendering
	VulkanDescriptorSet attachmentDescriptorSet;

	// Descriptor sets for shadow mapping
	VulkanDescriptorSet shadowMapDescriptorSet;

	// Shader modules
	VkShaderModule depthSkeletalVertShader;
	VkShaderModule depthSkeletalFragShader;

	VkShaderModule skeletalVertShader;
	VkShaderModule skeletalFragShader;

	VkShaderModule geometryVertShader;
	VkShaderModule geometryFragShader;

	VkShaderModule shadingVertShader;
	VkShaderModule shadingFragShader;

	VkShaderModule shadowVertShader;
	VkShaderModule shadowGeomShader;
	VkShaderModule shadowFragShader;

	VkShaderModule depthVertShader;
	VkShaderModule depthFragShader;

	VkShaderModule filterBrightCompShader;
	VkShaderModule downscaleCompShader;
	VkShaderModule upscaleCompShader;
	VkShaderModule blendColorCompShader;

	// ======================================
	VulkanAllocatedImage shadowCubeMap;
	VkSampler shadowSampler;

	VulkanDescriptorSet lightMatrixDescriptorSet;
	VulkanAllocatedMemory lightMatrixUniformBuffer;

	// ======================================

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

	void createCommandPools(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, std::vector<VkCommandPool>& commandPools);

	void createCommandBuffers(VkDevice& logicalDevice);
	void createSecondaryCommandBuffers(VkDevice& logicalDevice);

	void createSemaphore(VkDevice& logicalDevice);

	void createFence(VkDevice& logicalDevice, std::vector<VkFence>& fences, VkFenceCreateFlagBits flag);

	void createDescriptionPool(VkDevice& logicalDevice);

	void initUniformBuffer(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool, VulkanAllocatedMemory& uniformBuffer,VkDescriptorSet& descriptorSet, 
		VkDescriptorSetLayout& descriptorSetLayout, u32 binding, u32 bufferSize, VkShaderStageFlagBits shaderStage);
	void createUniformBuffer(VkDevice& logicalDevice, VulkanAllocatedMemory& uniformBuffer, u32 bufferSize);

	// Initialise descriptor set layout for skeletal rendering
	void initSkeletalDescriptorSetLayout(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool);

	// Initialise descriptor sets for shadow mapping
	void initShadowMapDescriptors(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool);

	// Initialise descriptor sets for deferred rendering
	void initAttachmentDescriptors(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool);

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