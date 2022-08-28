#pragma once
struct QueueFamilyIndices
{
	std::optional<u32> graphicsFamily;
	std::optional<u32> presentFamily;

	bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

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

	// Triple buffering
	u32 numSwapchainImage = 3;

public:

	VkInstance instance;

	VkPhysicalDevice physicalDevice;

	VkDevice logicalDevice;

	VkQueue graphicsQueue;

	VkQueue presentQueue;

	VkSurfaceKHR surface;

	VkSwapchainKHR swapchain;

	std::vector<VkImage> swapchainImages;

	VkFormat swapchainImageFormat;

	VkExtent2D swapchainExtent;

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

	void createSwapchain();

	void getSwapchainImages();

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

	std::optional<u32> findQueueFamilies(VkPhysicalDevice& device, VkQueueFlagBits flag, VkSurfaceKHR surface);

	bool checkDeviceExtensionSupport(VkPhysicalDevice& device);	

	void querySupportedSurfaceFormat(VkPhysicalDevice& device, VkSurfaceKHR& surface,
		VkSurfaceCapabilitiesKHR& capabilities, std::vector<VkSurfaceFormatKHR>& surfaceFormats, std::vector<VkPresentModeKHR>& presentModes);

	VkSurfaceFormatKHR selectSwapchainSurfaceFormat(std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats);

	VkPresentModeKHR selectSwapchainPresentMode(std::vector<VkPresentModeKHR>& presentModes);

	VkExtent2D getSwapchainExtent(VkSurfaceCapabilitiesKHR& capabilities);

	void printPhysicalDeviceInfo(VkPhysicalDevice& device);
};