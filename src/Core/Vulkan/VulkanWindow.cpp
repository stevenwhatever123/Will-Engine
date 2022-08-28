#include "pch.h"
#include "Core/Vulkan/VulkanWindow.h"

VulkanWindow::VulkanWindow() :
    enableValidationLayers(true),
    instance(VK_NULL_HANDLE),
    physicalDevice(VK_NULL_HANDLE),
    swapchainImages(),
    swapchainImageViews()
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

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSetWindowUserPointer(window, this);

    if (glfwVulkanSupported())
    {
        createInstance();
        setDebugMessage();
        createSurface();
        selectPhysicalDevice();
        createLogicalDevice();
        createSwapchain();
        getSwapchainImages();
        createSwapchainImageViews();

        initVulkanEngine();
    }
}

void VulkanWindow::cleanup()
{
    closeWindow = true;

    vulkanEngine->cleanup(logicalDevice);

    for (auto imageView : swapchainImageViews)
    {
        vkDestroyImageView(logicalDevice, imageView, nullptr);
    }

    vkDestroySwapchainKHR(logicalDevice, swapchain, nullptr);

    vkDestroyDevice(logicalDevice, nullptr);

    vkDestroySurfaceKHR(instance, surface, nullptr);

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
    if(!findQueueFamilies(physicalDevice, VK_QUEUE_GRAPHICS_BIT, VK_NULL_HANDLE).has_value())
        throw std::runtime_error("GPU does not have required Graphics Queue Family");

    if (!findQueueFamilies(physicalDevice, VK_QUEUE_GRAPHICS_BIT, surface))
        throw std::runtime_error("GPU does not have required Present Queue Family");

    u32 queueFamilyIndicies = findQueueFamilies(physicalDevice, VK_QUEUE_GRAPHICS_BIT, VK_NULL_HANDLE).value();
    u32 presentFamilyIndicies = findQueueFamilies(physicalDevice, VK_QUEUE_GRAPHICS_BIT, surface).value();

    f32 queuePriorities = 1.0f;

    // Device Features
    VkPhysicalDeviceFeatures deviceFeatures{};

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

    if (enableValidationLayers)
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

void VulkanWindow::createSwapchain()
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> availableSurfaceFormats;
    std::vector<VkPresentModeKHR> presentModes;

    querySupportedSurfaceFormat(physicalDevice, surface, capabilities, availableSurfaceFormats, presentModes);

    VkExtent2D extent = getSwapchainExtent(capabilities);
    // Store extent for later use
    swapchainExtent = extent;
    VkSurfaceFormatKHR surfaceFormat = selectSwapchainSurfaceFormat(availableSurfaceFormats);
    // Store image format for later use
    swapchainImageFormat = surfaceFormat.format;
    VkPresentModeKHR presentMode = selectSwapchainPresentMode(presentModes);

    // Make sure we have enough image for swapchain and make sure it does not exceed the maximum number
    if (numSwapchainImage < capabilities.minImageCount)
        throw std::runtime_error("Number of swapchain desired is less than the minimum required");
    if (numSwapchainImage > capabilities.maxImageCount)
        throw std::runtime_error("Number of swapchain desired is more than the maximum supported");

    VkSwapchainCreateInfoKHR swapchainInfo{};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = surface;
    swapchainInfo.minImageCount = numSwapchainImage;
    swapchainInfo.imageFormat = surfaceFormat.format;
    swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainInfo.imageExtent = extent;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    std::optional<u32> graphicsFamilyIndicies = findQueueFamilies(physicalDevice, VK_QUEUE_GRAPHICS_BIT, VK_NULL_HANDLE);
    std::optional<u32> presentFamilyIndicies = findQueueFamilies(physicalDevice, VK_QUEUE_GRAPHICS_BIT, surface);

    if (graphicsFamilyIndicies != presentFamilyIndicies)
    {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainInfo.queueFamilyIndexCount = 2;
        u32 queueFamilyIndicies[] = { graphicsFamilyIndicies.value(), presentFamilyIndicies.value() };
        swapchainInfo.pQueueFamilyIndices = queueFamilyIndicies;
    }
    else
    {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainInfo.queueFamilyIndexCount = 1;
    }

    swapchainInfo.preTransform = capabilities.currentTransform;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = presentMode;
    swapchainInfo.clipped = VK_TRUE;


    if (vkCreateSwapchainKHR(logicalDevice, &swapchainInfo, nullptr, &swapchain) != VK_SUCCESS)
        throw std::runtime_error("Failed to create swapchain");
}

void VulkanWindow::getSwapchainImages()
{
    u32 imageCount = 0;
    vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, nullptr);

    swapchainImages.resize(imageCount);

    vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, swapchainImages.data());
}

void VulkanWindow::createSwapchainImageViews()
{
    swapchainImageViews.resize(swapchainImages.size());

    for (u32 i = 0; i < swapchainImageViews.size(); i++)
    {
        createImageView(swapchainImages[i], swapchainImageViews[i], swapchainImageFormat);
    }
}


void VulkanWindow::initVulkanEngine()
{
    vulkanEngine = new VulkanEngine();
    vulkanEngine->init(instance, physicalDevice, logicalDevice, swapchainImageFormat);
}

void VulkanWindow::createPipeline()
{

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

bool VulkanWindow::isDeviceSuitable(VkPhysicalDevice& device, VkSurfaceKHR& surface)
{
    // Check if the device have the queue family we need
    if (!findQueueFamilies(physicalDevice, VK_QUEUE_GRAPHICS_BIT, VK_NULL_HANDLE).has_value())
        throw std::runtime_error("GPU does not have required Graphics Queue Family");

    // Check if the device have the extensions we need
    if (!checkDeviceExtensionSupport(physicalDevice))
        throw std::runtime_error("GPU does not have required extensions");

    // Check if the device has format and present mode support
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR> presentModes;
    querySupportedSurfaceFormat(device, surface, capabilities, surfaceFormats, presentModes);

    if (surfaceFormats.empty() || presentModes.empty())
        throw std::runtime_error("GPU does not have any surface formats or present modes available");

    return true;
}

std::optional<u32> VulkanWindow::findQueueFamilies(VkPhysicalDevice& device, VkQueueFlagBits flag, VkSurfaceKHR surface)
{
    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

    for (u32 i = 0; i < queueFamilyProperties.size(); i++)
    {
        if (queueFamilyProperties[i].queueFlags & flag)
        {
            if(surface == VK_NULL_HANDLE)
                return i;

            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

            if (presentSupport)
            {
                return i;
            }
        }
    }

    return {};
}

bool VulkanWindow::checkDeviceExtensionSupport(VkPhysicalDevice& device)
{
    u32 extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(physicalDeviceExtensions.begin(),
        physicalDeviceExtensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

void VulkanWindow::querySupportedSurfaceFormat(VkPhysicalDevice& device, 
    VkSurfaceKHR& surface,
    VkSurfaceCapabilitiesKHR &capabilities,
    std::vector<VkSurfaceFormatKHR>& surfaceFormats, 
    std::vector<VkPresentModeKHR>& presentModes)
{
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);

    u32 surfaceFormatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surfaceFormatCount, nullptr);

    if (surfaceFormatCount != 0)
    {
        surfaceFormats.resize(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surfaceFormatCount, surfaceFormats.data());
    }

    u32 presentModesCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, nullptr);

    if (presentModesCount != 0)
    {
        presentModes.resize(presentModesCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, presentModes.data());
    }
}

VkSurfaceFormatKHR VulkanWindow::selectSwapchainSurfaceFormat(std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats)
{
    for (const auto& availableFormat : availableSurfaceFormats)
    {
        if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    // Return the first element if none of them satisfie the requirement    
    return availableSurfaceFormats[0];
}

VkPresentModeKHR VulkanWindow::selectSwapchainPresentMode(std::vector<VkPresentModeKHR>& presentModes)
{
    for (const auto& availableMode : presentModes)
    {
        if (availableMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availableMode;
        }
    }

    // Use FIFO if MAILBOX is not availabe
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanWindow::getSwapchainExtent(VkSurfaceCapabilitiesKHR& capabilities)
{
    i32 width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D currentExtent;
    currentExtent.width = static_cast<u32>(width);
    currentExtent.height = static_cast<u32>(height);

    currentExtent.width = std::clamp(currentExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    currentExtent.height = std::clamp(currentExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return currentExtent;
}

void VulkanWindow::createImageView(VkImage& image, VkImageView& imageView, VkFormat format)
{
    VkImageViewCreateInfo imageViewInfo{};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = image;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = format;
    imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.layerCount = 1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.levelCount = 1;

    if (vkCreateImageView(logicalDevice, &imageViewInfo, nullptr, &imageView) != VK_SUCCESS)
        throw std::runtime_error("Failed to create image view");
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