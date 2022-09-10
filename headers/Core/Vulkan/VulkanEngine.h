#pragma once
#include "Core/Mesh.h"
#include "Core/Camera.h"

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

	std::vector<Mesh*> meshes;

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
	VkImageView depthImageView;
	VkImage depthImage;
	VmaAllocation depthImageAllocation;

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

	// Descriptor sets
	VkDescriptorSetLayout sceneDescriptorSetLayout;
	VkDescriptorSet sceneDescriptorSet;

	// Scene Uniform buffer
	VulkanAllocatedMemory sceneUniformBuffer;

	// Pipeline and Piplien Layout
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

private:

	mat4 sceneMatrix;

public:

	VulkanEngine();
	~VulkanEngine();

	void init(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR surface);
	void cleanup(VkDevice& logicalDevice);

	void update(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR surface, VkQueue graphicsQueue);

	// Init / setup

	void createVmaAllocator(VkInstance& instance, VkPhysicalDevice& physicalDevice, VkDevice& logicalDevice);

	void createSwapchain(GLFWwindow* window, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);
	void getSwapchainImages(VkDevice& logicalDevice);
	void createSwapchainImageViews(VkDevice& logicalDevice);

	void createRenderPass(VkDevice& logicalDevice, VkFormat& format, const VkFormat& depthFormat);

	void createDepthBuffer(VkDevice& logicalDevice, VmaAllocator& vmaAllocator, const VkExtent2D& swapchainExtent);

	void createSwapchainFramebuffer(VkDevice& logicalDevice, std::vector<VkImageView>& swapchainImageViews,
		std::vector<VkFramebuffer>& framebuffers, VkRenderPass& renderPass, VkImageView& depthImageView, VkExtent2D swapchainExtent);

	void createCommandPool(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkCommandPool& commandPool);

	void createCommandBuffer(VkDevice& logicalDevice, std::vector<VkCommandBuffer>& commandBuffers);

	void createSemaphore(VkDevice& logicalDevice, VkSemaphore& waitImageAvailable, VkSemaphore& signalRenderFinish);

	void createFence(VkDevice& logicalDevice, std::vector<VkFence>& fences, VkFenceCreateFlagBits flag);

	void createDescriptionPool(VkDevice& logicalDevice);

	void createSceneUniformBuffers(VkDevice& logicalDevice, VulkanAllocatedMemory& uniformBuffer);

	void updateSceneDescriptorSet(VkDevice& logicalDevice, VkDescriptorSet& descriptorSet, VkBuffer& descriptorBuffer);

	// Update

	void updateSceneUniform(Camera* camera);

	void recordCommands(VkCommandBuffer& commandBuffer, VkRenderPass& renderpass, VkFramebuffer& framebuffer, VkExtent2D& extent);

	void submitCommands(VkCommandBuffer& commandBuffer, VkSemaphore& waitSemaphore, VkSemaphore& signalSemaphore, 
		VkQueue& graphicsQueue, VkFence& fence);

	void presentImage(VkQueue& graphicsQueue, VkSemaphore& waitSemaphore, VkSwapchainKHR& swapchain, u32& swapchainIndex);


private:
	VkSurfaceFormatKHR selectSwapchainSurfaceFormat(std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats);

	VkPresentModeKHR selectSwapchainPresentMode(std::vector<VkPresentModeKHR>& presentModes);
};