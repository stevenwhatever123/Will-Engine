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

std::tuple<VkBuffer, VmaAllocation> WillEngine::VulkanUtil::createBuffer(VmaAllocator& vmaAllocator, u64 allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = allocSize;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo vmaAllocInfo{};
    vmaAllocInfo.usage = memoryUsage;

    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation vmaAllocation = VK_NULL_HANDLE;

    if (vmaCreateBuffer(vmaAllocator, &bufferInfo, &vmaAllocInfo, &buffer, &vmaAllocation, nullptr) != VK_SUCCESS)
        throw std::runtime_error("Failed to create buffer");

    return std::make_tuple(std::move(buffer), std::move(vmaAllocation));
}

VkCommandPool WillEngine::VulkanUtil::createCommandPool(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface)
{
    std::optional<u32> graphicsFamilyIndex = WillEngine::VulkanUtil::findQueueFamilies(physicalDevice, VK_QUEUE_GRAPHICS_BIT, surface);

    if (!graphicsFamilyIndex.has_value())
        throw std::runtime_error("Cannot retrieve graphics family index");

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = graphicsFamilyIndex.value();

    VkCommandPool commandPool = VK_NULL_HANDLE;

    if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create command pool");

    return std::move(commandPool);
}

VkCommandBuffer WillEngine::VulkanUtil::createCommandBuffer(VkDevice& logicalDevice, VkCommandPool& commandPool)
{
    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    if (vkAllocateCommandBuffers(logicalDevice, &allocateInfo, &commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffers");

    return std::move(commandBuffer);
}

VkFence WillEngine::VulkanUtil::createFence(VkDevice& logicalDevice, bool signaled)
{
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if(signaled)
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkFence fence = VK_NULL_HANDLE;

    if (vkCreateFence(logicalDevice, &fenceInfo, nullptr, &fence) != VK_SUCCESS)
        throw std::runtime_error("Failed to create fence");

    return std::move(fence);
}

void WillEngine::VulkanUtil::bufferBarrier(VkCommandBuffer& commandBuffer, VkBuffer& buffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
    VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkDeviceSize size, VkDeviceSize offset, u32 srcFamilyIndex, u32 dstFamilyIndex)
{
    VkBufferMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.srcAccessMask = srcAccessMask;
    barrier.dstAccessMask = dstAccessMask;
    barrier.srcQueueFamilyIndex = srcFamilyIndex;
    barrier.dstQueueFamilyIndex = dstFamilyIndex;
    barrier.buffer = buffer;
    barrier.offset = offset;
    barrier.size = size;

    vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 1, &barrier, 0, nullptr);
}

VkShaderModule WillEngine::VulkanUtil::createShaderModule(VkDevice& logicalDevice, std::vector<char>& shaderCode)
{
    VkShaderModuleCreateInfo shaderInfo{};
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = shaderCode.size();
    shaderInfo.pCode = reinterpret_cast<const u32*>(shaderCode.data());

    VkShaderModule shaderModule = VK_NULL_HANDLE;

    if (vkCreateShaderModule(logicalDevice, &shaderInfo, nullptr, &shaderModule) != VK_SUCCESS)
        throw std::runtime_error("Failed to create shader module");

    return shaderModule;
}

void WillEngine::VulkanUtil::createDescriptorSetLayout(VkDevice& logicalDevice, VkDescriptorSetLayout& descriptorSetLayout,
    VkDescriptorType descriptorType, VkShaderStageFlags shaderStage)
{
    VkDescriptorSetLayoutBinding binding[1]{};
    binding[0].binding = 0;
    binding[0].descriptorType = descriptorType;
    binding[0].descriptorCount = 1;
    binding[0].stageFlags = shaderStage;

    VkDescriptorSetLayoutCreateInfo descriptorInfo{};
    descriptorInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorInfo.bindingCount = sizeof(binding) / sizeof(binding[0]);
    descriptorInfo.pBindings = binding;

    if (vkCreateDescriptorSetLayout(logicalDevice, &descriptorInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor set layout");
}

void WillEngine::VulkanUtil::allocDescriptorSet(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool, VkDescriptorSetLayout& descriptorSetInfo,
    VkDescriptorSet& descriptorSet)
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetInfo;

    if (vkAllocateDescriptorSets(logicalDevice, &allocInfo, &descriptorSet) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate Descriptor Sets");
}