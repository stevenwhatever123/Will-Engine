#include "pch.h"
#include "Core/Vulkan/VulkanWindow.h"

VulkanWindow::VulkanWindow() :
    enableValidationLayers(true),
    instance(VK_NULL_HANDLE),
    physicalDevice(VK_NULL_HANDLE)
{

}

VulkanWindow::~VulkanWindow()
{

}

void VulkanWindow::init()
{
    closeWindow = false;

    if (!glfwInit())
        closeWindow = true;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(1600, 900, "Will Engine - Vulkan", nullptr, nullptr);

    if (glfwVulkanSupported())
    {
        createInstance();
        setDebugMessage();
        createSurface();
        selectPhysicalDevice();
        createLogicalDevice();
    }

    cleanup();
}

void VulkanWindow::cleanup()
{
    closeWindow = true;

    destroyDebugMessage();

    vkDestroyDevice(logicalDevice, nullptr);

    vkDestroySurfaceKHR(instance, surface, nullptr);

    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}

void VulkanWindow::update()
{

}

void VulkanWindow::createInstance()
{
    if (volkInitialize() != VK_SUCCESS)
        throw std::runtime_error("Failed to initialize volk\n");

    // Create Vulkan instance if vulkan is supported
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Will Engine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Will Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    // Get all required extensions
    std::vector<const char*> requiredExtensions = getRequiredExtensions();

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<u32>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    // Validation layer for debugging

    if (enableValidationLayers && !checkValidationLayerSupport(validationLayers))
        throw std::runtime_error("Validation requested but not available!\n");

    if (enableValidationLayers)
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

    //printf("Available extensions:\n");

    //for (const auto& extension : suppportedExtensions)
    //{
    //    printf("\t %s\n", extension.extensionName);
    //}
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
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT debugMessageInfo{};
    generateDebugMessageInfo(debugMessageInfo);

    // Create Debug callback
    if (vkCreateDebugUtilsMessengerEXT(instance, &debugMessageInfo, nullptr, &debugMessenger)
        != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to set up debug messenger!\n");
    }
}

void VulkanWindow::destroyDebugMessage()
{
    if (!enableValidationLayers) return;

    vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
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
    if (!findQueueFamilies(physicalDevice).graphicsFamily.has_value())
        throw std::runtime_error("GPU does not have required Graphics Queue Family\n");

    printPhysicalDeviceInfo(physicalDevice);
}

void VulkanWindow::createLogicalDevice()
{
    QueueFamilyIndices queueFamilyIndicies = findQueueFamilies(physicalDevice);

    f32 queuePriorities = 1.0f;

    VkPhysicalDeviceFeatures deviceFeatures{};

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    std::vector<u32> uniqueQueueFaimiles;
    if (queueFamilyIndicies.graphicsFamily.value() == queueFamilyIndicies.presentFamily.value())
    {
        uniqueQueueFaimiles.push_back(queueFamilyIndicies.graphicsFamily.value());
    }
    else
    {
        uniqueQueueFaimiles.push_back(queueFamilyIndicies.graphicsFamily.value());
        uniqueQueueFaimiles.push_back(queueFamilyIndicies.presentFamily.value());
    }

    for (u32& queueFamily : uniqueQueueFaimiles)
    {
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueCount = 1;
        queueInfo.queueFamilyIndex = queueFamilyIndicies.graphicsFamily.value();
        queueInfo.pQueuePriorities = &queuePriorities;
        queueCreateInfos.push_back(queueInfo);
    }

    VkDeviceCreateInfo logicalDeviceInfo{};
    logicalDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logicalDeviceInfo.queueCreateInfoCount = static_cast<u32>(uniqueQueueFaimiles.size());
    logicalDeviceInfo.pQueueCreateInfos = queueCreateInfos.data();
    logicalDeviceInfo.pEnabledFeatures = &deviceFeatures;

    if (enableValidationLayers)
    {
        logicalDeviceInfo.enabledLayerCount = static_cast<u32>(validationLayers.size());
        logicalDeviceInfo.ppEnabledLayerNames = validationLayers.data();
    }

    if (vkCreateDevice(physicalDevice, &logicalDeviceInfo, nullptr, &logicalDevice) != VK_SUCCESS)
        throw std::runtime_error("Failed to create vulkan logical device\n");

    // Retrive the graphics queue for later use
    vkGetDeviceQueue(logicalDevice, queueFamilyIndicies.graphicsFamily.value(), 0, &graphicsQueue);

    // Retrive the present queue for later use
    vkGetDeviceQueue(logicalDevice, queueFamilyIndicies.presentFamily.value(), 0, &presentQueue);
}

void VulkanWindow::createSurface()
{
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("Failed to create window surface");
}

//=============================================================================================
// Private calls you wouldn't normally need outside of this class

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

    if (enableValidationLayers)
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

QueueFamilyIndices VulkanWindow::findQueueFamilies(VkPhysicalDevice& device)
{
    QueueFamilyIndices indices;

    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

    for (u32 i = 0; i < queueFamilyProperties.size(); i++)
    {
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

            if (presentSupport)
            {
                indices.graphicsFamily = i;
                indices.presentFamily = i;
            }
        }
    }

    return indices;
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