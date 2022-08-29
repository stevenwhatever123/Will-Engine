#include "pch.h"
#include "Utils/VulkanUtil.h"

std::optional<u32> WillEngine::VulkanUtil::findQueueFamilies(VkPhysicalDevice& physicalDevice, VkQueueFlagBits flag, VkSurfaceKHR surface)
{
    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    for (u32 i = 0; i < queueFamilyProperties.size(); i++)
    {
        if (queueFamilyProperties[i].queueFlags & flag)
        {
            if (surface == VK_NULL_HANDLE)
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

void WillEngine::VulkanUtil::querySupportedSurfaceFormat(VkPhysicalDevice& device,
    VkSurfaceKHR& surface,
    VkSurfaceCapabilitiesKHR& capabilities,
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

bool WillEngine::VulkanUtil::checkDeviceExtensionSupport(VkPhysicalDevice& device, const std::vector<const char*>& physicalDeviceExtensions)
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

VkExtent2D WillEngine::VulkanUtil::getSwapchainExtent(GLFWwindow* window, VkSurfaceCapabilitiesKHR& capabilities)
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

void WillEngine::VulkanUtil::createImageView(VkDevice& logicalDevice, VkImage& image, VkImageView& imageView, VkFormat format, VkImageAspectFlags aspectMask)
{
    VkImageViewCreateInfo imageViewInfo{};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = image;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = format;
    imageViewInfo.components = VkComponentMapping{};
    imageViewInfo.subresourceRange.aspectMask = aspectMask;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.layerCount = 1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.levelCount = 1;

    //VK_IMAGE_ASPECT_COLOR_BIT

    if (vkCreateImageView(logicalDevice, &imageViewInfo, nullptr, &imageView) != VK_SUCCESS)
        throw std::runtime_error("Failed to create image view");
}