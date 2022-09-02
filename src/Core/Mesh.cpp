#include "pch.h"
#include "Core/Mesh.h"

Mesh::Mesh():
	name(""),
	positions(),
	normals(),
	uvs(),
	indicies(),
	transformation(1),
	primitive(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
{

}

Mesh::~Mesh()
{

}

void Mesh::sendDataToGPU(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkSurfaceKHR& surface, VkQueue& queue)
{
	// Buffer that is going to send to the GPU
	std::tie(vertexBuffer, vertexAllocation) = WillEngine::VulkanUtil::createBuffer(vmaAllocator, sizeof(vec3) * positions.size(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	std::tie(normalBuffer, normalAllocation) = WillEngine::VulkanUtil::createBuffer(vmaAllocator, sizeof(vec3) * normals.size(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	std::tie(uvBuffer, uvAllocation) = WillEngine::VulkanUtil::createBuffer(vmaAllocator, sizeof(vec2) * uvs.size(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	std::tie(indexBuffer, indexAllocation) = WillEngine::VulkanUtil::createBuffer(vmaAllocator, sizeof(u32) * indicies.size(),
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);


	// Staging buffers
	VkBuffer vertexStagingBuffer;
	VmaAllocation vertexStagingAllocation;
	std::tie(vertexStagingBuffer, vertexStagingAllocation) = WillEngine::VulkanUtil::createBuffer(vmaAllocator, sizeof(vec3) * positions.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	VkBuffer normalStagingBuffer;
	VmaAllocation normalStagingAllocation;
	std::tie(normalStagingBuffer, normalStagingAllocation) = WillEngine::VulkanUtil::createBuffer(vmaAllocator, sizeof(vec3) * normals.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	VkBuffer uvStagingBuffer;
	VmaAllocation uvStagingAllocation;
	std::tie(uvStagingBuffer, uvStagingAllocation) = WillEngine::VulkanUtil::createBuffer(vmaAllocator, sizeof(vec2) * uvs.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	VkBuffer indexStagingBuffer;
	VmaAllocation indexStagingAllocation;
	std::tie(indexStagingBuffer, indexStagingAllocation) = WillEngine::VulkanUtil::createBuffer(vmaAllocator, sizeof(u32) * indicies.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	// Copy data to the staging buffer first
	void* positionPtr = nullptr;
	if (vmaMapMemory(vmaAllocator, vertexStagingAllocation, &positionPtr) != VK_SUCCESS)
		throw std::runtime_error("Failed to map memory");
	std::memcpy(positionPtr, positions.data(), sizeof(vec3) * positions.size());
	vmaUnmapMemory(vmaAllocator, vertexStagingAllocation);

	void* normalPtr = nullptr;
	if (vmaMapMemory(vmaAllocator, normalStagingAllocation, &normalPtr) != VK_SUCCESS)
		throw std::runtime_error("Failed to map memory");
	std::memcpy(normalPtr, normals.data(), sizeof(vec3) * normals.size());
	vmaUnmapMemory(vmaAllocator, normalStagingAllocation);

	void* uvPtr = nullptr;
	if (vmaMapMemory(vmaAllocator, uvStagingAllocation, &uvPtr) != VK_SUCCESS)
		throw std::runtime_error("Failed to map memory");
	std::memcpy(uvPtr, uvs.data(), sizeof(vec2) * uvs.size());
	vmaUnmapMemory(vmaAllocator, uvStagingAllocation);

	void* indexPtr = nullptr;
	if (vmaMapMemory(vmaAllocator, indexStagingAllocation, &indexPtr) != VK_SUCCESS)
		throw std::runtime_error("Failed to map memory");
	std::memcpy(indexPtr, indicies.data(), sizeof(u32) * indicies.size());
	vmaUnmapMemory(vmaAllocator, indexStagingAllocation);

	VkFence uploadComplete = WillEngine::VulkanUtil::createFence(logicalDevice, false);

	VkCommandPool commandPool = WillEngine::VulkanUtil::createCommandPool(logicalDevice, physicalDevice, surface);
	VkCommandBuffer commandBuffer = WillEngine::VulkanUtil::createCommandBuffer(logicalDevice, commandPool);

	VkCommandBufferBeginInfo commandInfo{};
	commandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandInfo.flags = 0;

	if (vkBeginCommandBuffer(commandBuffer, &commandInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to begin command buffer");

	// Copy buffer from staging to the actual one
	VkBufferCopy positionCopy{};
	positionCopy.size = sizeof(vec3) * positions.size();
	vkCmdCopyBuffer(commandBuffer, vertexStagingBuffer, vertexBuffer, 1, &positionCopy);

	WillEngine::VulkanUtil::bufferBarrier(commandBuffer, vertexBuffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_WHOLE_SIZE, 0, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);

	VkBufferCopy normalCopy{};
	normalCopy.size = sizeof(vec3) * normals.size();
	vkCmdCopyBuffer(commandBuffer, normalStagingBuffer, normalBuffer, 1, &normalCopy);

	WillEngine::VulkanUtil::bufferBarrier(commandBuffer, normalBuffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_WHOLE_SIZE, 0, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);

	VkBufferCopy uvCopy{};
	uvCopy.size = sizeof(vec2) * uvs.size();
	vkCmdCopyBuffer(commandBuffer, uvStagingBuffer, uvBuffer, 1, &uvCopy);

	WillEngine::VulkanUtil::bufferBarrier(commandBuffer, uvBuffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_WHOLE_SIZE, 0, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);

	VkBufferCopy indexCopy{};
	indexCopy.size = sizeof(u32) * indicies.size();
	vkCmdCopyBuffer(commandBuffer, indexStagingBuffer, indexBuffer, 1, &indexCopy);

	WillEngine::VulkanUtil::bufferBarrier(commandBuffer, indexBuffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_WHOLE_SIZE, 0, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);

	// End Command buffer
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to end command buffer");

	// Submit the recorded commands
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;


	if (vkQueueSubmit(queue, 1, &submitInfo, uploadComplete) != VK_SUCCESS)
		throw std::runtime_error("Failed to submit commands");

	if (vkWaitForFences(logicalDevice, 1, &uploadComplete, VK_TRUE, std::numeric_limits<u64>::max()) != VK_SUCCESS)
		throw std::runtime_error("Failed to wait for fence");

	// Now we can clean data from the CPU as we don't need it anymore
}

void Mesh::generateDescriptorSet(VmaAllocator vmaAllocator, u32 queueIndiciesSize, u32* queueIndices)
{
	
}

void Mesh::generatePipeline(VkDevice& logicalDevice, VkRenderPass& renderpass, VkExtent2D swapchainExtent)
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
	pipelineInfo.layout = 0;
	pipelineInfo.renderPass = renderpass;
	pipelineInfo.subpass = 0;

	if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
		throw std::runtime_error("Failed to create graphics pipeline");
}

void Mesh::cleanup(VkDevice& logicalDevice)
{
	vkDestroyShaderModule(logicalDevice, vertShader, nullptr);
	vkDestroyShaderModule(logicalDevice, fragShader, nullptr);
}