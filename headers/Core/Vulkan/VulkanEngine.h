#pragma once
#include "Core/Camera.h"
#include "Core/Mesh.h"
#include "Core/Material.h"
#include "Core/Light.h"
#include "Core/Camera.h"
#include "Core/UniformClass.h"

#include "Core/Vulkan/VulkanFramebuffer.h"
#include "Core/Vulkan/VulkanGui.h"

#include "Utils/VulkanUtil.h"

class VulkanEngine
{
private:

public:

	bool updateTexture = false;
	bool updateColor = false;
	u32 selectedMaterialIndex = 0;
	u32 selectedTextureIndex = 0;
	std::string textureFilepath = "";

	Camera* camera;

	std::vector<Mesh*> meshes;
	std::vector<Material*> materials;

	std::vector<Light*> lights;

public:

	// Triple buffering
	const u32 numSwapchainImage = 3;

	const VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

	const VkFormat shadowDepthFormat = VK_FORMAT_D32_SFLOAT;

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

	VkExtent2D swapchainExtent;

	// Depth buffer
	VulkanAllocatedImage depthImage;
	VkImageView depthImageView;

	// Shadow map framebuffer
	VkFramebuffer shadowFrameBuffer;

	// Framebuffer for depth pre-pass
	VkFramebuffer depthFrameBuffer;

	VulkanAllocatedImage shadingImage;
	VkFramebuffer shadingFrameBuffer;
	VkImageView shadingImageView;
	VkDescriptorSet imguiRenderedDescriptorSet;

	// Framebuffer
	std::vector<VkFramebuffer> framebuffers;
	// Stored as a vector as there are multiple framebuffers in a swapchain
	VulkanFramebuffer offscreenFramebuffer;

	// Sampler to sample framebuffer's color attachment
	VkSampler attachmentSampler;

	// Command pool and buffer
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;

	// Semaphore for waiting and signaling
	// Used for GPU - GPU sync
	VkSemaphore imageAvailable;
	VkSemaphore renderFinished;

	// Fence 
	// Used for CPU - GPU symc
	std::vector<VkFence> fences;

	VkDescriptorPool descriptorPool;

	// Pipeline and pipeline layout (Blinn Phong Shader)
	VkPipelineLayout geometryPipelineLayout;
	VkPipeline geometryPipeline;
	// Pipeline for Shading
	VkPipelineLayout shadingPipelineLayout;
	VkPipeline shadingPipeline;
	// Pipeline for shadow
	VkPipelineLayout shadowPipelineLayout;
	VkPipeline shadowPipeline;
	// Pipeline for depth pre pass
	VkPipelineLayout depthPipelineLayout;
	VkPipeline depthPipeline;

	// Scene Descriptor sets
	VkDescriptorSetLayout sceneDescriptorSetLayout;
	VkDescriptorSet sceneDescriptorSet;
	// Scene Uniform buffer
	VulkanAllocatedMemory sceneUniformBuffer;

	// Light Descriptor sets
	VkDescriptorSetLayout lightDescriptorSetLayout;
	VkDescriptorSet lightDescriptorSet;
	// Lights Uniform buffer
	VulkanAllocatedMemory lightUniformBuffer;

	// Camera Descriptor sets
	VkDescriptorSetLayout cameraDescriptorSetLayout;
	VkDescriptorSet cameraDescriptorSet;
	// Camera Uniform buffer
	VulkanAllocatedMemory cameraUniformBuffer;

	// Texture Descriptor sets
	VkDescriptorSetLayout textureDescriptorSetLayout;

	// Descriptor sets for shading from deferred rendering
	VkDescriptorSetLayout attachmentDescriptorSetLayouts;
	VkDescriptorSet attachmentDescriptorSets;

	// Descriptor sets for shadow mapping
	VkDescriptorSetLayout shadowMapDescriptorSetLayouts;
	VkDescriptorSet shadowMapDescriptorSets;

	// Shader modules
	VkShaderModule geometryVertShader;
	VkShaderModule geometryFragShader;

	VkShaderModule shadingVertShader;
	VkShaderModule shadingFragShader;

	VkShaderModule shadowVertShader;
	VkShaderModule shadowGeomShader;
	VkShaderModule shadowFragShader;

	VkShaderModule depthVertShader;
	VkShaderModule depthFragShader;

	// ======================================
	VulkanAllocatedImage shadowCubeMap;
	VkImageView shadowCubeMapView;
	VkSampler shadowSampler;

	VkDescriptorSetLayout lightMatrixDescriptorSetLayout;
	VkDescriptorSet lightMatrixDescriptorSet;
	VulkanAllocatedMemory lightMatrixUniformBuffer;

	// ======================================

	// GUI
	VulkanGui* vulkanGui;

private:

	CameraMatrix sceneMatrix;

public:

	VulkanEngine();
	~VulkanEngine();

	void init(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR surface, VkQueue& queue, 
		bool renderWithBRDF);
	void cleanup(VkDevice& logicalDevice);

	void update(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR surface, VkQueue graphicsQueue, bool renderWithBRDF);

	// Init / setup

	void createVmaAllocator(VkInstance& instance, VkPhysicalDevice& physicalDevice, VkDevice& logicalDevice);

	void createSwapchain(GLFWwindow* window, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);
	void getSwapchainImages(VkDevice& logicalDevice);
	void createSwapchainImageViews(VkDevice& logicalDevice);
	void recreateSwapchain(GLFWwindow* window, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);

	void createDepthPrePass(VkDevice& logicalDevice, VkRenderPass& renderPass, const VkFormat& depthFormat);
	void createPresentRenderPass(VkDevice& logicalDevice, VkRenderPass& renderPass, const VkFormat& format);
	void createShadingRenderPass(VkDevice& logicalDevice, VkRenderPass& renderPass, VkFormat& format, const VkFormat& depthFormat);
	void createShadowRenderPass(VkDevice& logicalDevice, VkRenderPass& renderPass, const VkFormat& depthFormat);
	void createGeometryRenderPass(VkDevice& logicalDevice, VkRenderPass& renderPass, VkFormat format, const VkFormat& depthFormat);

	void createDepthBuffer(VkDevice& logicalDevice, VmaAllocator& vmaAllocator, const VkExtent2D& swapchainExtent);

	void createSwapchainFramebuffer(VkDevice& logicalDevice, std::vector<VkImageView>& swapchainImageViews,
		std::vector<VkFramebuffer>& framebuffers, VulkanFramebuffer& offscreenFramebuffer, VkRenderPass& geometryRenderPass, VkRenderPass& renderPass, VkImageView& depthImageView, 
		VkExtent2D swapchainExtent);

	void createShadowFramebuffer(VkDevice& logicalDevice, VkFramebuffer& shadowFramebuffer, VkRenderPass& shadowRenderPass, u32 width, u32 height);

	void createShadingFramebuffer(VkDevice& logicalDevice, VkFramebuffer& shadingFramebuffer, VkRenderPass& shadingRenderPass, VkExtent2D extent);

	void createDepthFramebuffer(VkDevice& logicalDevice, VkFramebuffer& depthFramebuffer, VkRenderPass& depthRenderPass, VkExtent2D swapchainExtent);

	void createCommandPool(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkCommandPool& commandPool);

	void createCommandBuffer(VkDevice& logicalDevice, std::vector<VkCommandBuffer>& commandBuffers);

	void createSemaphore(VkDevice& logicalDevice, VkSemaphore& waitImageAvailable, VkSemaphore& signalRenderFinish);

	void createFence(VkDevice& logicalDevice, std::vector<VkFence>& fences, VkFenceCreateFlagBits flag);

	void createDescriptionPool(VkDevice& logicalDevice);

	void initUniformBuffer(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool, VulkanAllocatedMemory& uniformBuffer,VkDescriptorSet& descriptorSet, 
		VkDescriptorSetLayout& descriptorSetLayout, u32 binding, u32 bufferSize, VkShaderStageFlagBits shaderStage);
	void createUniformBuffer(VkDevice& logicalDevice, VulkanAllocatedMemory& uniformBuffer, u32 bufferSize);

	// Initialise descriptor sets for shadow mapping
	void initShadowMapDescriptors(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool);

	// Initialise descriptor sets for deferred rendering
	void initAttachmentDescriptors(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool);

	void initRenderedDescriptors(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool);

	// Pipeline init
	void initGeometryPipeline(VkDevice& logicalDevice);
	void initDepthPipeline(VkDevice& logicalDevice);
	void initShadowPipeline(VkDevice& logicalDevice);
	void initShadingPipeline(VkDevice& logicalDevice);

	// GUI
	void initGui(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkQueue& queue, VkSurfaceKHR& surface);

	void updateGui(VkDevice& logicalDevice, VkQueue& graphicsQueue, bool renderWithBRDF);

	// Update
	void updateSceneUniform(Camera* camera);
	void updateLightUniform(Camera* camera);

	void recordCommands(VkCommandBuffer& commandBuffer, VkFramebuffer& framebuffer, VkExtent2D& extent);

	// Render passes
	void depthPrePasses(VkCommandBuffer& commandBuffer, VkExtent2D extent);
	void geometryPasses(VkCommandBuffer& commandBuffer, VkExtent2D extent);
	void shadowPasses(VkCommandBuffer& commandBuffer);
	void shadingPasses(VkCommandBuffer& commandBuffer, VkRenderPass& renderPass, VkFramebuffer& framebuffer, VkExtent2D extent);
	void UIPasses(VkCommandBuffer& commandBuffer, VkRenderPass& renderPass, VkFramebuffer& framebuffer, VkExtent2D extent);

	void submitCommands(VkCommandBuffer& commandBuffer, VkSemaphore& waitSemaphore, VkSemaphore& signalSemaphore, 
		VkQueue& graphicsQueue, VkFence& fence);

	void presentImage(VkQueue& graphicsQueue, VkSemaphore& waitSemaphore, VkSwapchainKHR& swapchain, u32& swapchainIndex);


private:
	VkSurfaceFormatKHR selectSwapchainSurfaceFormat(std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats);

	VkPresentModeKHR selectSwapchainPresentMode(std::vector<VkPresentModeKHR>& presentModes);

	void changeMaterialTexture(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue, bool& updateTexture, 
		bool& updateColor, u32 materialIndex, u32 textureIndex, std::string filename);
};