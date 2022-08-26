#include "pch.h"
#include "Core/Vulkan/VulkanWindow.h"

VulkanWindow::VulkanWindow() :
    enableValidationLayers(true)
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
    }

    closeWindow = true;

    destroyDebugMessage();

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
    const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

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

    printf("Available extensions:\n");

    for (const auto& extension : suppportedExtensions)
    {
        printf("\t %s\n", extension.extensionName);
    }
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