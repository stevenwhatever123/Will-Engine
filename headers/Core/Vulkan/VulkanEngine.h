#pragma once
#include "Core/Camera.h"
#include "Core/Mesh.h"
#include "Core/Material.h"
#include "Core/Light.h"
#include "Core/Camera.h"

#include "Core/Vulkan/VulkanFramebuffer.h"
#include "Core/Vulkan/VulkanGui.h"

#include "Utils/VulkanUtil.h"

struct cameraProject
{
	mat4 cameraMatrix;
	mat4 projectionMatrix;
};

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

public:

	VmaAllocator vmaAllocator;

	VkRenderPass deferredRenderPass;
	VkRenderPass renderPass;

	// Swapchain
	VkSwapchainKHR swapchain;

	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;

	VkFormat swapchainImageFormat;

	VkExtent2D swapchainExtent;

	// Depth buffer
	VulkanAllocatedImage depthImage;
	VkImageView depthImageView;

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
	VkPipelineLayout defaultPipelineLayout;
	VkPipeline defaultPipeline;
	// BRDF Metallic
	VkPipelineLayout brdfMetallicPipelineLayout;
	VkPipeline brdfMetallicPipeline;
	// Pipeline for 
	VkPipelineLayout combinePipelineLayout;
	VkPipeline combinePipeline;

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

	// Descriptor sets for deferred rendering
	VkDescriptorSetLayout attachmentDescriptorSetLayouts;
	VkDescriptorSet attachmentDescriptorSets;

	// Shader modules
	VkShaderModule defaultVertShader;
	VkShaderModule defaultFragShader;

	VkShaderModule combineVertShader;
	VkShaderModule combineFragShader;

	// GUI
	VulkanGui* vulkanGui;

private:

	cameraProject sceneMatrix;

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

	void createRenderPass(VkDevice& logicalDevice, VkFormat& format, const VkFormat& depthFormat);
	void createDeferredRenderPass(VkDevice& logicalDevice, VkFormat format, const VkFormat& depthFormat);

	void createDepthBuffer(VkDevice& logicalDevice, VmaAllocator& vmaAllocator, const VkExtent2D& swapchainExtent);

	void createSwapchainFramebuffer(VkDevice& logicalDevice, std::vector<VkImageView>& swapchainImageViews,
		std::vector<VkFramebuffer>& framebuffers, VulkanFramebuffer& offscreenFramebuffer, VkRenderPass& deferredRenderPass, VkRenderPass& renderPass, VkImageView& depthImageView, 
		VkExtent2D swapchainExtent);

	void createCommandPool(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkCommandPool& commandPool);

	void createCommandBuffer(VkDevice& logicalDevice, std::vector<VkCommandBuffer>& commandBuffers);

	void createSemaphore(VkDevice& logicalDevice, VkSemaphore& waitImageAvailable, VkSemaphore& signalRenderFinish);

	void createFence(VkDevice& logicalDevice, std::vector<VkFence>& fences, VkFenceCreateFlagBits flag);

	void createDescriptionPool(VkDevice& logicalDevice);

	void initUniformBuffer(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool, VulkanAllocatedMemory& uniformBuffer,VkDescriptorSet& descriptorSet, 
		VkDescriptorSetLayout& descriptorSetLayout, u32 binding, u32 bufferSize, VkShaderStageFlagBits shaderStage);
	void createUniformBuffer(VkDevice& logicalDevice, VulkanAllocatedMemory& uniformBuffer, u32 bufferSize);

	// Initialise descriptor sets for deferred rendering
	void initCombineDescriptors(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool);

	// GUI
	void initGui(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkQueue& queue, VkSurfaceKHR& surface);

	void updateGui(VkDevice& logicalDevice, VkQueue& graphicsQueue, bool renderWithBRDF);

	// Update
	void updateSceneUniform(Camera* camera);
	void updateLightUniform(Camera* camera);

	void recordCommands(VkCommandBuffer& commandBuffer, VkRenderPass& renderPass, VkFramebuffer& framebuffer, VkExtent2D& extent);

	// Render passes
	void renderPasses(VkCommandBuffer& commandBuffer, VkExtent2D extent);
	void shadingPasses(VkCommandBuffer& commandBuffer, VkRenderPass& renderpass, VkFramebuffer& framebuffer, VkExtent2D extent);
	void UIPasses(VkCommandBuffer& commandBuffer, VkExtent2D extent);

	void submitCommands(VkCommandBuffer& commandBuffer, VkSemaphore& waitSemaphore, VkSemaphore& signalSemaphore, 
		VkQueue& graphicsQueue, VkFence& fence);

	void presentImage(VkQueue& graphicsQueue, VkSemaphore& waitSemaphore, VkSwapchainKHR& swapchain, u32& swapchainIndex);


private:
	VkSurfaceFormatKHR selectSwapchainSurfaceFormat(std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats);

	VkPresentModeKHR selectSwapchainPresentMode(std::vector<VkPresentModeKHR>& presentModes);

	void changeMaterialTexture(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue, bool& updateTexture, 
		bool& updateColor, u32 materialIndex, u32 textureIndex, std::string filename);
};