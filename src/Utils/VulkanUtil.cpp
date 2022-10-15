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

VulkanAllocatedImage WillEngine::VulkanUtil::createImage(VkDevice& logicalDevice, VmaAllocator& vmaAllocator, VkImage& image, VkFormat format, u32 width, u32 height, u32 mipLevels)
{
    // Image Info
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = format;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // Allocate Memory
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VulkanAllocatedImage vulkanImage;

    if (vmaCreateImage(vmaAllocator, &imageInfo, &allocInfo, &vulkanImage.image, &vulkanImage.allocation, nullptr) != VK_SUCCESS)
        throw std::runtime_error("Failed to create image");

    return std::move(vulkanImage);
}

void WillEngine::VulkanUtil::loadTextureImage(VkDevice& logicalDevice, VmaAllocator vmaAllocator, VkCommandPool& commandPool, VkQueue& queue, 
    VulkanAllocatedImage& vulkanImage, u32 mipLevels, u32 width, u32 height, unsigned char* textureImage)
{
    VkCommandBuffer commandBuffer = createCommandBuffer(logicalDevice, commandPool);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin Command Buffer");

    imageBarrier(commandBuffer, vulkanImage.image, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT , 0, mipLevels, 0, 1 }, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    auto sizeInBytes = width * height * 4;

    // Staging Buffer
    VulkanAllocatedMemory stagingBuffer = createBuffer(vmaAllocator, sizeInBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    
    // Copying the texture to the staging buffer
    void* ptr = nullptr;
    if (vmaMapMemory(vmaAllocator, stagingBuffer.allocation, &ptr) != VK_SUCCESS)
        throw std::runtime_error("Vma failed to map memory");
    std::memcpy(ptr, textureImage, sizeInBytes);

    vmaUnmapMemory(vmaAllocator, stagingBuffer.allocation);

    // Copy data from staging to the actual buffer
    VkBufferImageCopy copy{};
    copy.bufferOffset = 0;
    copy.bufferRowLength = 0;
    copy.bufferImageHeight = 0;
    copy.imageSubresource = VkImageSubresourceLayers{
        VK_IMAGE_ASPECT_COLOR_BIT , 0, 0, 1
    };
    copy.imageOffset = VkOffset3D{ 0, 0, 0 };
    copy.imageExtent = VkExtent3D{ width, height, 1 };

    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.buffer, vulkanImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

    imageBarrier(commandBuffer, vulkanImage.image, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT , 0, mipLevels, 0, 1 }, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    // End Command Buffer
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to end command buffer");

    VkFence uploadComplete = createFence(logicalDevice, false);

    // Submit the recorded commands
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    if (vkQueueSubmit(queue, 1, &submitInfo, uploadComplete) != VK_SUCCESS)
        throw std::runtime_error("Failed to submit commands");

    if (vkWaitForFences(logicalDevice, 1, &uploadComplete, VK_TRUE, std::numeric_limits<u64>::max()) != VK_SUCCESS)
        throw std::runtime_error("Failed to wait for fence");

    // Clean up staging buffers
    vmaDestroyBuffer(vmaAllocator, stagingBuffer.buffer, stagingBuffer.allocation);

    // Clean up command buffer and fence
    vkDestroyFence(logicalDevice, uploadComplete, nullptr);
    vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
}

void WillEngine::VulkanUtil::loadTextureImageWithMipmap(VkDevice& logicalDevice, VmaAllocator vmaAllocator, VkCommandPool& commandPool, VkQueue& queue,
    VulkanAllocatedImage& vulkanImage, u32 mipLevels, u32 width, u32 height, unsigned char* textureImage)
{
    VkCommandBuffer commandBuffer = createCommandBuffer(logicalDevice, commandPool);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin Command Buffer");

    imageBarrier(commandBuffer, vulkanImage.image, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT , 0, mipLevels, 0, 1 }, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    // Create staging buffers for image upload
    auto sizeInBytes = width * height * 4;

    // Staging Buffer
    VulkanAllocatedMemory stagingBuffer = createBuffer(vmaAllocator, sizeInBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    // Copying the texture to the staging buffer
    void* ptr = nullptr;
    if (vmaMapMemory(vmaAllocator, stagingBuffer.allocation, &ptr) != VK_SUCCESS)
        throw std::runtime_error("Vma failed to map memory");
    std::memcpy(ptr, textureImage, sizeInBytes);

    vmaUnmapMemory(vmaAllocator, stagingBuffer.allocation);

    // Copy data from staging to the actual buffer
    VkBufferImageCopy copy{};
    copy.bufferOffset = 0;
    copy.bufferRowLength = 0;
    copy.bufferImageHeight = 0;
    copy.imageSubresource = VkImageSubresourceLayers{
        VK_IMAGE_ASPECT_COLOR_BIT , 0, 0, 1
    };
    copy.imageOffset = VkOffset3D{ 0, 0, 0 };
    copy.imageExtent = VkExtent3D{ width, height, 1 };

    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.buffer, vulkanImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

    imageBarrier(commandBuffer, vulkanImage.image, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT , 0, mipLevels, 0, 1 }, VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    i32 currentWidth = width;
    i32 currentHeight = height;

    // Compute each mip level
    for (u32 level = 1; level < mipLevels; level++)
    {
        imageBarrier(commandBuffer, vulkanImage.image, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT , level - 1, 1, 0, 1 }, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

        VkImageBlit imageBlit{};
        imageBlit.srcOffsets[0] = {0, 0, 0};
        imageBlit.srcOffsets[1] = { currentWidth, currentHeight, 1 };
        imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.srcSubresource.mipLevel = level - 1;
        imageBlit.srcSubresource.baseArrayLayer = 0;
        imageBlit.srcSubresource.layerCount = 1;
        imageBlit.dstOffsets[0] = {0, 0, 0};
        imageBlit.dstOffsets[1] = { currentWidth > 1 ? currentWidth / 2 : 1, currentHeight > 1 ? currentHeight / 2 : 1, 1 };
        imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.dstSubresource.mipLevel = level;
        imageBlit.dstSubresource.baseArrayLayer = 0;
        imageBlit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer, vulkanImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vulkanImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 
            &imageBlit, VK_FILTER_LINEAR);

        imageBarrier(commandBuffer, vulkanImage.image, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT , level - 1, 1, 0, 1 }, VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        if (currentWidth > 1) currentWidth /= 2;
        if (currentHeight > 1) currentHeight /= 2;
    }

    // Transition the barrier back from VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    imageBarrier(commandBuffer, vulkanImage.image, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT , 0, mipLevels, 0, 1 }, VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    // End Command Buffer
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to end command buffer");

    VkFence uploadComplete = createFence(logicalDevice, false);

    // Submit the recorded commands
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    if (vkQueueSubmit(queue, 1, &submitInfo, uploadComplete) != VK_SUCCESS)
        throw std::runtime_error("Failed to submit commands");

    if (vkWaitForFences(logicalDevice, 1, &uploadComplete, VK_TRUE, std::numeric_limits<u64>::max()) != VK_SUCCESS)
        throw std::runtime_error("Failed to wait for fence");

    // Clean up staging buffers
    vmaDestroyBuffer(vmaAllocator, stagingBuffer.buffer, stagingBuffer.allocation);

    // Clean up command buffer and fence
    vkDestroyFence(logicalDevice, uploadComplete, nullptr);
    vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
}

u32 WillEngine::VulkanUtil::calculateMiplevels(u32 width, u32 height)
{
    return static_cast<u32>(std::floor(std::log2(std::max(width, height)))) + 1;
}

void WillEngine::VulkanUtil::createImageView(VkDevice& logicalDevice, VkImage& image, VkImageView& imageView, u32 mipLevels, 
    VkFormat format, VkImageAspectFlags aspectMask)
{
    VkImageViewCreateInfo imageViewInfo{};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = image;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = format;
    imageViewInfo.components = VkComponentMapping{};
    imageViewInfo.subresourceRange.aspectMask = aspectMask;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.layerCount = VK_REMAINING_MIP_LEVELS;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.levelCount = mipLevels;

    //VK_IMAGE_ASPECT_COLOR_BIT

    if (vkCreateImageView(logicalDevice, &imageViewInfo, nullptr, &imageView) != VK_SUCCESS)
        throw std::runtime_error("Failed to create image view");
}

void WillEngine::VulkanUtil::createDefaultSampler(VkDevice& logicalDevice, VkSampler& sampler)
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

    if (vkCreateSampler(logicalDevice, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
        throw std::runtime_error("Failed to create sampler");
}

void WillEngine::VulkanUtil::createTextureSampler(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSampler& sampler, u32 mipLevels)
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<f32>(mipLevels);
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.compareEnable = VK_TRUE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    if (vkCreateSampler(logicalDevice, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
        throw std::runtime_error("Failed to create sampler");
}

VulkanAllocatedMemory WillEngine::VulkanUtil::createBuffer(VmaAllocator& vmaAllocator, u64 allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
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

    return VulkanAllocatedMemory(std::move(buffer), std::move(vmaAllocation));
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

void WillEngine::VulkanUtil::imageBarrier(VkCommandBuffer& commandBuffer, VkImage& image, VkAccessFlags srcAccessFlag, VkAccessFlags dstAccessFlag,
    VkImageLayout srcLayout, VkImageLayout dstLayout, VkImageSubresourceRange subresourceRange, u32 srcQueueFamilyIndex, u32 dstQueueFamilyIndex,
    VkPipelineStageFlags srcStageFlag, VkPipelineStageFlags dstStageFlag)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcAccessMask = srcAccessFlag;
    barrier.dstAccessMask = dstAccessFlag;
    barrier.oldLayout = srcLayout;
    barrier.newLayout = dstLayout;
    barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
    barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
    barrier.image = image;
    barrier.subresourceRange = subresourceRange;

    vkCmdPipelineBarrier(commandBuffer, srcStageFlag, dstStageFlag, 0, 0, nullptr, 0, nullptr, 1, &barrier);
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

void WillEngine::VulkanUtil::initPhongShaderModule(VkDevice& logicalDevice, VkShaderModule& vertShader, VkShaderModule& fragShader)
{
    const char* vertShaderPath = "C:/Users/Steven/source/repos/Will-Engine/shaders/compiled_shaders/shader.vert.spv";
    const char* fragShaderPath = "C:/Users/Steven/source/repos/Will-Engine/shaders/compiled_shaders/shader.frag.spv";

    auto vertShaderCode = WillEngine::Utils::readSprivShader(vertShaderPath);
    auto fragShaderCode = WillEngine::Utils::readSprivShader(fragShaderPath);

    vertShader = WillEngine::VulkanUtil::createShaderModule(logicalDevice, vertShaderCode);
    fragShader = WillEngine::VulkanUtil::createShaderModule(logicalDevice, fragShaderCode);
}

void WillEngine::VulkanUtil::initBRDFShaderModule(VkDevice& logicalDevice, VkShaderModule& vertShader, VkShaderModule& fragShader)
{
    const char* vertShaderPath = "C:/Users/Steven/source/repos/Will-Engine/shaders/compiled_shaders/shader.vert.spv";
    const char* fragShaderPath = "C:/Users/Steven/source/repos/Will-Engine/shaders/compiled_shaders/pbrShader.frag.spv";

    auto vertShaderCode = WillEngine::Utils::readSprivShader(vertShaderPath);
    auto fragShaderCode = WillEngine::Utils::readSprivShader(fragShaderPath);

    vertShader = WillEngine::VulkanUtil::createShaderModule(logicalDevice, vertShaderCode);
    fragShader = WillEngine::VulkanUtil::createShaderModule(logicalDevice, fragShaderCode);
}

void WillEngine::VulkanUtil::createDescriptorSetLayout(VkDevice& logicalDevice, VkDescriptorSetLayout& descriptorSetLayout,
    VkDescriptorType descriptorType, VkShaderStageFlags shaderStage, u32 binding, u32 descriptorCount)
{
    VkDescriptorSetLayoutBinding layoutBinding[1]{};
    layoutBinding[0].binding = binding;
    layoutBinding[0].descriptorType = descriptorType;
    layoutBinding[0].descriptorCount = descriptorCount;
    layoutBinding[0].stageFlags = shaderStage;

    VkDescriptorSetLayoutCreateInfo descriptorInfo{};
    descriptorInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorInfo.bindingCount = sizeof(layoutBinding) / sizeof(layoutBinding[0]);
    descriptorInfo.pBindings = layoutBinding;

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

void WillEngine::VulkanUtil::writeDescriptorSetBuffer(VkDevice& logicalDevice, VkDescriptorSet& descriptorSet, VkBuffer& descriptorBuffer, u32 binding)
{
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = descriptorBuffer;
    bufferInfo.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet writeSet{};
    writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSet.dstSet = descriptorSet;
    writeSet.dstBinding = binding;
    writeSet.descriptorCount = 1;
    writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeSet.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(logicalDevice, 1, &writeSet, 0, nullptr);
}

void WillEngine::VulkanUtil::writeDescriptorSetImage(VkDevice& logicalDevice, VkDescriptorSet& descriptorSet, std::vector<VkSampler>& sampler, 
    std::vector<VkImageView>& imageView, VkImageLayout imageLayout, u32 binding, u32 descriptorCount)
{
    std::vector<VkDescriptorImageInfo> imageInfos(descriptorCount);
    for (u32 i = 0; i < descriptorCount; i++)
    {
        imageInfos[i].sampler = sampler[i];
        imageInfos[i].imageView = imageView[i];
        imageInfos[i].imageLayout = imageLayout;
    }

    VkWriteDescriptorSet writeSet{};
    writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSet.dstSet = descriptorSet;
    writeSet.dstBinding = binding;
    writeSet.descriptorCount = descriptorCount;
    writeSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeSet.pImageInfo = imageInfos.data();

    vkUpdateDescriptorSets(logicalDevice, 1, &writeSet, 0, nullptr);
}

void WillEngine::VulkanUtil::createPipelineLayout(VkDevice& logicalDevice, VkPipelineLayout& pipelineLayout, u32 size,
    VkDescriptorSetLayout* descriptorSetLayout, u32 pushConstantCount, VkPushConstantRange* pushConstant)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = size;
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayout;

    pipelineLayoutInfo.pPushConstantRanges = pushConstant;
    pipelineLayoutInfo.pushConstantRangeCount = pushConstantCount;
    
    if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline layout");
}

void WillEngine::VulkanUtil::createPipeline(VkDevice& logicalDevice, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, VkRenderPass& renderpass, 
    VkShaderModule& vertShader, VkShaderModule& fragShader, VkPrimitiveTopology primitive, VkExtent2D swapchainExtent)
{
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShader;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShader;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo stages[] = { vertShaderStageInfo , fragShaderStageInfo };

    // Shader code inputs
    // Position
    VkVertexInputBindingDescription vertexInputs[3]{};
    vertexInputs[0].binding = 0;
    vertexInputs[0].stride = sizeof(vec3);
    vertexInputs[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    // Normal
    vertexInputs[1].binding = 1;
    vertexInputs[1].stride = sizeof(vec3);
    vertexInputs[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    // UV
    vertexInputs[2].binding = 2;
    vertexInputs[2].stride = sizeof(vec2);
    vertexInputs[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vertexAttrib[3]{};
    // Position
    vertexAttrib[0].location = 0;
    vertexAttrib[0].binding = 0;
    vertexAttrib[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttrib[0].offset = 0;
    // Normal
    vertexAttrib[1].location = 1;
    vertexAttrib[1].binding = 1;
    vertexAttrib[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttrib[1].offset = 0;
    // UV
    vertexAttrib[2].location = 2;
    vertexAttrib[2].binding = 2;
    vertexAttrib[2].format = VK_FORMAT_R32G32_SFLOAT;
    vertexAttrib[2].offset = 0;

    // Input Info
    VkPipelineVertexInputStateCreateInfo inputInfo{};
    inputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    inputInfo.vertexBindingDescriptionCount = sizeof(vertexInputs) / sizeof(vertexInputs[0]);
    inputInfo.pVertexBindingDescriptions = vertexInputs;
    inputInfo.vertexAttributeDescriptionCount = sizeof(vertexAttrib) / sizeof(vertexAttrib[0]);
    inputInfo.pVertexAttributeDescriptions = vertexAttrib;

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = primitive;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewport
    VkViewport viewport{};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = swapchainExtent.width;
    viewport.height = swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Scissor
    VkRect2D scissor{};
    scissor.extent = swapchainExtent;
    scissor.offset = { 0, 0 };

    VkPipelineViewportStateCreateInfo viewportInfo{};
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.viewportCount = 1;
    viewportInfo.pViewports = &viewport;
    viewportInfo.scissorCount = 1;
    viewportInfo.pScissors = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
    rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerInfo.depthClampEnable = VK_FALSE;
    rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizerInfo.depthBiasEnable = VK_FALSE;
    rasterizerInfo.lineWidth = 1.0f;

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampleInfo{};
    multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleInfo.sampleShadingEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlend[1]{};
    colorBlend[0].blendEnable = VK_FALSE;
    colorBlend[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
    colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo.logicOpEnable = VK_FALSE;
    colorBlendInfo.attachmentCount = 1;
    colorBlendInfo.pAttachments = colorBlend;

    // Depth test
    VkPipelineDepthStencilStateCreateInfo depthInfo{};
    depthInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthInfo.depthTestEnable = VK_TRUE;
    depthInfo.depthWriteEnable = VK_TRUE;
    depthInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthInfo.minDepthBounds = 0.0f;
    depthInfo.maxDepthBounds = 1.0f;

    // Create graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = stages;
    pipelineInfo.pVertexInputState = &inputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportInfo;
    pipelineInfo.pRasterizationState = &rasterizerInfo;
    pipelineInfo.pMultisampleState = &multisampleInfo;
    pipelineInfo.pDepthStencilState = &depthInfo;
    pipelineInfo.pColorBlendState = &colorBlendInfo;
    pipelineInfo.pDynamicState = nullptr;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderpass;
    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
        throw std::runtime_error("Failed to create graphics pipeline");
}