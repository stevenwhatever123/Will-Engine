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

void WillEngine::VulkanUtil::createPipelineLayout(VkDevice& logicalDevice, VkPipelineLayout& pipelineLayout, u32 size,
    VkDescriptorSetLayout* descriptorSetLayout)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = size;
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayout;

    if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline layout");
}

void WillEngine::VulkanUtil::createPipeline(VkDevice& logicalDevice, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, VkRenderPass& renderpass, 
    VkShaderModule& vertShader, VkShaderModule& fragShader, VkPrimitiveTopology& primitive, VkExtent2D swapchainExtent)
{
    // Shader Module
    //const char* vertShaderPath = "shaders/compiled_shaders/shader.vert.spv";
    //const char* fragShaderPath = "shaders/compiled_shaders/shader.frag.spv";

    const char* vertShaderPath = "C:/Users/Steven/Documents/GitHub/Will-Engine/shaders/compiled_shaders/shader.vert.spv";
    const char* fragShaderPath = "C:/Users/Steven/Documents/GitHub/Will-Engine/shaders/compiled_shaders/shader.frag.spv";


    auto vertShaderCode = WillEngine::Utils::readSprivShader(vertShaderPath);
    auto fragShaderCode = WillEngine::Utils::readSprivShader(fragShaderPath);

    vertShader = WillEngine::VulkanUtil::createShaderModule(logicalDevice, vertShaderCode);
    fragShader = WillEngine::VulkanUtil::createShaderModule(logicalDevice, fragShaderCode);

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