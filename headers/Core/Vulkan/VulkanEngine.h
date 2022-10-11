#pragma once
#include "Core/Camera.h"
#include "Core/Mesh.h"
#include "Core/Material.h"
#include "Core/Light.h"
#include "Core/Camera.h"

#include "Core/Vulkan/VulkanGui.h"

#include "Utils/VulkanUtil.h"

struct RendObj
{
	u32 startIndex;
	u32 endIndex;
	VkPrimitiveTopology primitive;
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
	// Stored as a vector as there are multiple framebuffers in a swapchain
	std::vector<VkFramebuffer> framebuffers;

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

	// Pipeline and pipeline layout
	VkPipelineLayout defaultPipelineLayout;
	VkPipeline defaultPipeline;

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

	// Shader modules
	VkShaderModule defaultVertShader;
	VkShaderModule defaultFragShader;

	// GUI
	VulkanGui* vulkanGui;

private:

	mat4 sceneMatrix;

public:

	VulkanEngine();
	~VulkanEngine();

	void init(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR surface, VkQueue& queue);
	void cleanup(VkDevice& logicalDevice);

	void update(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR surface, VkQueue graphicsQueue);

	// Init / setup

	void createVmaAllocator(VkInstance& instance, VkPhysicalDevice& physicalDevice, VkDevice& logicalDevice);

	void createSwapchain(GLFWwindow* window, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);
	void getSwapchainImages(VkDevice& logicalDevice);
	void createSwapchainImageViews(VkDevice& logicalDevice);
	void recreateSwapchain(GLFWwindow* window, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);

	void createRenderPass(VkDevice& logicalDevice, VkFormat& format, const VkFormat& depthFormat);

	void createDepthBuffer(VkDevice& logicalDevice, VmaAllocator& vmaAllocator, const VkExtent2D& swapchainExtent);

	void createSwapchainFramebuffer(VkDevice& logicalDevice, std::vector<VkImageView>& swapchainImageViews,
		std::vector<VkFramebuffer>& framebuffers, VkRenderPass& renderPass, VkImageView& depthImageView, VkExtent2D swapchainExtent);

	void createCommandPool(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkCommandPool& commandPool);

	void createCommandBuffer(VkDevice& logicalDevice, std::vector<VkCommandBuffer>& commandBuffers);

	void createSemaphore(VkDevice& logicalDevice, VkSemaphore& waitImageAvailable, VkSemaphore& signalRenderFinish);

	void createFence(VkDevice& logicalDevice, std::vector<VkFence>& fences, VkFenceCreateFlagBits flag);

	void createDescriptionPool(VkDevice& logicalDevice);

	void initUniformBuffer(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool, VulkanAllocatedMemory& uniformBuffer,VkDescriptorSet& descriptorSet, 
		VkDescriptorSetLayout& descriptorSetLayout, u32 binding, u32 bufferSize, VkShaderStageFlagBits shaderStage);
	void createUniformBuffer(VkDevice& logicalDevice, VulkanAllocatedMemory& uniformBuffer, u32 bufferSize);

	// GUI
	void initGui(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkQueue& queue, VkSurfaceKHR& surface);

	void updateGui(VkDevice& logicalDevice, VkQueue& graphicsQueue);

	// Update
	void updateSceneUniform(Camera* camera);
	void updateLightUniform(Camera* camera);

	void recordCommands(VkCommandBuffer& commandBuffer, VkRenderPass& renderPass, VkFramebuffer& framebuffer, VkExtent2D& extent);

	void renderPasses(VkCommandBuffer& commandBuffer);
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