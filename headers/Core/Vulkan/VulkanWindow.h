#pragma once
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

	VkInstance instance;

	VkPhysicalDevice physicalDevice;

	VkDevice logicalDevice;

	VkQueue graphicsQueue;

	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

public:

	VulkanWindow();
	~VulkanWindow();

	void init();
	void update();

	void createInstance();

	void generateDebugMessageInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
	void setDebugMessage();
	void destroyDebugMessage();

	void selectPhysicalDevice();

	void createLogicalDevice();

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

	u32 findQueueFamilies(VkPhysicalDevice& device);

	void printPhysicalDeviceInfo(VkPhysicalDevice& device);
};