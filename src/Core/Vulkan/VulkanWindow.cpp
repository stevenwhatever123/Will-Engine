#include "pch.h"
#include "Core/Vulkan/VulkanWindow.h"

VulkanWindow::VulkanWindow() :
    m_enableValidationLayers(true),
    m_debugMessenger(VK_NULL_HANDLE),
    window(nullptr),
    closeWindow(false),
    windowWidth(0),
    windowHeight(0),
    imguiWidth(0),
    imguiHeight(0),
    instance(VK_NULL_HANDLE),
    physicalDevice(VK_NULL_HANDLE),
    logicalDevice(VK_NULL_HANDLE),
    graphicsQueue(VK_NULL_HANDLE),
    presentQueue(VK_NULL_HANDLE),
    surface(VK_NULL_HANDLE),
    vulkanEngine(nullptr)
{

}

VulkanWindow::~VulkanWindow()
{

}

void VulkanWindow::createWindow(i32 windowWidth, i32 windowHeight)
{
    closeWindow = false;

    if (!glfwInit())
        closeWindow = true;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(windowWidth, windowHeight, "Will Engine - Vulkan", nullptr, nullptr);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
}

void VulkanWindow::initVulkan()
{
    if (glfwVulkanSupported())
    {
        createInstance();
        setDebugMessage();
        createSurface();
        selectPhysicalDevice();
        createLogicalDevice();

        initVulkanEngine();
    }
}

void VulkanWindow::cleanup()
{
    closeWindow = true;

    // Clean up everthing in the vulkan engine
    vulkanEngine->cleanup(logicalDevice);

    // Destroy logical device
    vkDestroyDevice(logicalDevice, nullptr);

    // Destroy surface
    vkDestroySurfaceKHR(instance, surface, nullptr);

    // Destroy debug message
    destroyDebugMessage();

    // Destroy instance
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}

void VulkanWindow::update()
{
    vulkanEngine->update(window, instance, logicalDevice, physicalDevice, surface, graphicsQueue);
}

void VulkanWindow::createInstance()
{
    if (volkInitialize() != VK_SUCCESS)
        throw std::runtime_error("Failed to initialize volk\n");

    // Create Vulkan instance if vulkan is supported
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Will Engine";
    appInfo.applicationVersion = 1;
    appInfo.apiVersion = VK_API_VERSION_1_2;

    // Get all required extensions
    std::vector<const char*> requiredExtensions = getRequiredExtensions();

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<u32>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    // Validation layer for debugging
    if (m_enableValidationLayers && !checkValidationLayerSupport(validationLayers))
        throw std::runtime_error("Validation requested but not available!\n");

    if (m_enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<u32>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        // Debug Message Info
        VkDebugUtilsMessengerCreateInfoEXT debugMessageInfo{};
        generateDebugMessageInfo(debugMessageInfo);
        
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugMessageInfo;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
        throw std::runtime_error("Failed to create vulkan instance\n");

    volkLoadInstance(instance);

    // Check supported extensions on this system
    u32 supportedExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr);

    std::vector<VkExtensionProperties> suppportedExtensions(supportedExtensionCount);

    vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, suppportedExtensions.data());
}

void VulkanWindow::generateDebugMessageInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

void VulkanWindow::setDebugMessage()
{
    if (!m_enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT debugMessageInfo{};
    generateDebugMessageInfo(debugMessageInfo);

    // Create Debug callback
    if (vkCreateDebugUtilsMessengerEXT(instance, &debugMessageInfo, nullptr, &m_debugMessenger)
        != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to set up debug messenger!\n");
    }
}

void VulkanWindow::destroyDebugMessage()
{
    if (!m_enableValidationLayers) return;

    vkDestroyDebugUtilsMessengerEXT(instance, m_debugMessenger, nullptr);
}

void VulkanWindow::selectPhysicalDevice()
{
    u32 availablePhysicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &availablePhysicalDeviceCount, nullptr);

    if (availablePhysicalDeviceCount == 0)
        throw std::runtime_error("Unable to find GPU with vulkan support\n");

    std::vector<VkPhysicalDevice> availablePhysicalDevices(availablePhysicalDeviceCount);
    vkEnumeratePhysicalDevices(instance, &availablePhysicalDeviceCount, availablePhysicalDevices.data());

    u32 score = 0;

    // Pick a physical device based on score
    for (auto& availablePhysicalDevice : availablePhysicalDevices)
    {
        u32 deviceScore = getDeviceScore(availablePhysicalDevice);

        if (deviceScore > score)
        {
            score = deviceScore;
            physicalDevice = availablePhysicalDevice;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("Failed to find a suitable GPU\n");

    // Check if the device is suitable
    
    if (!isDeviceSuitable(physicalDevice, surface))
        throw std::runtime_error("GPU is not suitable");

    printPhysicalDeviceInfo(physicalDevice);
}

void VulkanWindow::createLogicalDevice()
{
    // Make sure we have the queue family we needed
    if(!WillEngine::VulkanUtil::findQueueFamilies(physicalDevice, VK_QUEUE_GRAPHICS_BIT, VK_NULL_HANDLE).has_value())
        throw std::runtime_error("GPU does not have required Graphics Queue Family");

    if (!WillEngine::VulkanUtil::findQueueFamilies(physicalDevice, VK_QUEUE_GRAPHICS_BIT, surface))
        throw std::runtime_error("GPU does not have required Present Queue Family");

    u32 queueFamilyIndicies = WillEngine::VulkanUtil::findQueueFamilies(physicalDevice, VK_QUEUE_GRAPHICS_BIT, VK_NULL_HANDLE).value();
    u32 presentFamilyIndicies = WillEngine::VulkanUtil::findQueueFamilies(physicalDevice, VK_QUEUE_GRAPHICS_BIT, surface).value();

    f32 queuePriorities = 1.0f;

    // Device Features
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    std::vector<u32> uniqueQueueFaimiles;
    if (queueFamilyIndicies == presentFamilyIndicies)
    {
        uniqueQueueFaimiles.push_back(queueFamilyIndicies);
    }
    else
    {
        uniqueQueueFaimiles.insert(uniqueQueueFaimiles.end(),
            { queueFamilyIndicies,  presentFamilyIndicies });
    }

    // Create queue infos based on the queue family
    for (u32& queueFamily : uniqueQueueFaimiles)
    {
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueCount = 1;
        queueInfo.queueFamilyIndex = queueFamilyIndicies;
        queueInfo.pQueuePriorities = &queuePriorities;
        queueCreateInfos.push_back(queueInfo);
    }

    VkDeviceCreateInfo logicalDeviceInfo{};
    logicalDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logicalDeviceInfo.queueCreateInfoCount = static_cast<u32>(uniqueQueueFaimiles.size());
    logicalDeviceInfo.pQueueCreateInfos = queueCreateInfos.data();
    logicalDeviceInfo.pEnabledFeatures = &deviceFeatures;
    // Enable physical device extension
    logicalDeviceInfo.enabledExtensionCount = static_cast<u32>(physicalDeviceExtensions.size());
    logicalDeviceInfo.ppEnabledExtensionNames = physicalDeviceExtensions.data();

    if (m_enableValidationLayers)
    {
        logicalDeviceInfo.enabledLayerCount = static_cast<u32>(validationLayers.size());
        logicalDeviceInfo.ppEnabledLayerNames = validationLayers.data();
    }

    if (vkCreateDevice(physicalDevice, &logicalDeviceInfo, nullptr, &logicalDevice) != VK_SUCCESS)
        throw std::runtime_error("Failed to create vulkan logical device\n");

    // Retrive the graphics queue for later use
    vkGetDeviceQueue(logicalDevice, queueFamilyIndicies, 0, &graphicsQueue);

    // Retrive the present queue for later use
    vkGetDeviceQueue(logicalDevice, presentFamilyIndicies, 0, &presentQueue);
}

void VulkanWindow::createSurface()
{
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("Failed to create window surface");
}

void VulkanWindow::initVulkanEngine()
{
    vulkanEngine = new VulkanEngine();
    vulkanEngine->init(window, instance, logicalDevice, physicalDevice, surface, graphicsQueue);
}

//=============================================================================================
// Private helper functions you wouldn't normally want to know outside of the class

bool VulkanWindow::checkValidationLayerSupport(const std::vector<const char*> validationLayers)
{
    u32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return false;
    }

    return true;
}

std::vector<const char*> VulkanWindow::getRequiredExtensions()
{
    u32 glfwExtentionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtentionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtentionCount);

    if (m_enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanWindow::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT  messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        printf("Validation Layer: %s\n", pCallbackData->pMessage);
    }

    return VK_FALSE;
}

u32 VulkanWindow::getDeviceScore(VkPhysicalDevice& device)
{
    // Rating the device by score
    u32 score = 0;

    VkPhysicalDeviceProperties deviceProperties{};
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures{};
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 100;
    else
        score += 10;

    return score;
}

bool VulkanWindow::isDeviceSuitable(VkPhysicalDevice& device, VkSurfaceKHR& surface)
{
    // Check if the device have the queue family we need
    if (!WillEngine::VulkanUtil::findQueueFamilies(physicalDevice, VK_QUEUE_GRAPHICS_BIT, VK_NULL_HANDLE).has_value())
        throw std::runtime_error("GPU does not have required Graphics Queue Family");

    // Check if the device have the extensions we need
    if (!WillEngine::VulkanUtil::checkDeviceExtensionSupport(physicalDevice, physicalDeviceExtensions))
        throw std::runtime_error("GPU does not have required extensions");

    // Check if the device has format and present mode support
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR> presentModes;
    WillEngine::VulkanUtil::querySupportedSurfaceFormat(device, surface, capabilities, surfaceFormats, presentModes);

    if (surfaceFormats.empty() || presentModes.empty())
        throw std::runtime_error("GPU does not have any surface formats or present modes available");

    return true;
}

void VulkanWindow::printPhysicalDeviceInfo(VkPhysicalDevice& device)
{
    VkPhysicalDeviceProperties deviceProperties{};
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    printf("Device Name: %s\n", deviceProperties.deviceName);
    printf("Vulkan API Version: %u.%u.%u\n", 
        VK_API_VERSION_MAJOR(deviceProperties.apiVersion),
        VK_API_VERSION_MINOR(deviceProperties.apiVersion),
        VK_API_VERSION_PATCH(deviceProperties.apiVersion)
    );
}