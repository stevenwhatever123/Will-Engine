#pragma once
#include "Core/Vulkan/VulkanEngine.h"

#include "Utils/VulkanUtil.h"

class VulkanWindow
{
private:

	bool enableValidationLayers;

	// For debugging
	VkDebugUtilsMessengerEXT debugMessenger;

public:

	GLFWwindow* window;

	bool closeWindow;

	i32 windowWidth, windowHeight;

	i32 imguiWidth, imguiHeight;

public:

	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

	const std::vector<const char*> physicalDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

public:

	VkInstance instance;

	VkPhysicalDevice physicalDevice;

	VkDevice logicalDevice;

	VkQueue graphicsQueue;

	VkQueue presentQueue;

	VkSurfaceKHR surface;

	// The majority vulkan stuffs run here
	VulkanEngine* vulkanEngine;

public:

	VulkanWindow();
	~VulkanWindow();

	void init();
	void cleanup();
	void update();

	void createInstance();

	void generateDebugMessageInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
	void setDebugMessage();
	void destroyDebugMessage();

	void selectPhysicalDevice();

	void createLogicalDevice();

	void createSurface();

	void initVulkanEngine();

	void createPipeline();

	// Return
	bool shouldCloseWindow() const { return closeWindow; };

private:

	bool checkValidationLayerSupport(const std::vector<const char*> validationLayers);
	std::vector<const char*> getRequiredExtensions();

	// Debug callback
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT  messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	);

	u32 getDeviceScore(VkPhysicalDevice& device);

	bool isDeviceSuitable(VkPhysicalDevice& device, VkSurfaceKHR& surface);

	void printPhysicalDeviceInfo(VkPhysicalDevice& device);
};