#include "pch.h"
#include "Core/SkinnedMesh.h"

SkinnedMesh::SkinnedMesh():
	Mesh(),
	boneWeights(),
	boneIdsBuffer({VK_NULL_HANDLE, VK_NULL_HANDLE}),
	weightsBuffer({VK_NULL_HANDLE, VK_NULL_HANDLE})
{

}

SkinnedMesh::~SkinnedMesh()
{

}

void SkinnedMesh::uploadDataToPhysicalDevice(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkSurfaceKHR& surface, VkQueue& queue)
{
	// Buffer that is going to send to the GPU
	positionBuffer = WillEngine::VulkanUtil::createBuffer(vmaAllocator, sizeof(vec3) * positions.size(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	normalBuffer = WillEngine::VulkanUtil::createBuffer(vmaAllocator, sizeof(vec3) * normals.size(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	tangentBuffer = WillEngine::VulkanUtil::createBuffer(vmaAllocator, sizeof(vec3) * tangents.size(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	uvBuffer = WillEngine::VulkanUtil::createBuffer(vmaAllocator, sizeof(vec2) * uvs.size(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	boneIdsBuffer = WillEngine::VulkanUtil::createBuffer(vmaAllocator, sizeof(ivec4) * boneWeights.size(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	weightsBuffer = WillEngine::VulkanUtil::createBuffer(vmaAllocator, sizeof(vec4) * boneWeights.size(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	indexBuffer = WillEngine::VulkanUtil::createBuffer(vmaAllocator, sizeof(u32) * indicies.size(),
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);


	// Staging buffers
	VulkanAllocatedMemory positionStagingBuffer{};
	positionStagingBuffer = WillEngine::VulkanUtil::createBuffer(vmaAllocator, sizeof(vec3) * positions.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	VulkanAllocatedMemory normalStagingBuffer{};
	normalStagingBuffer = WillEngine::VulkanUtil::createBuffer(vmaAllocator, sizeof(vec3) * normals.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	VulkanAllocatedMemory tangentStagingBuffer{};
	tangentStagingBuffer = WillEngine::VulkanUtil::createBuffer(vmaAllocator, sizeof(vec3) * tangents.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	VulkanAllocatedMemory uvStagingBuffer{};
	uvStagingBuffer = WillEngine::VulkanUtil::createBuffer(vmaAllocator, sizeof(vec2) * uvs.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	VulkanAllocatedMemory boneIdsStagingBuffer;
	boneIdsStagingBuffer = WillEngine::VulkanUtil::createBuffer(vmaAllocator, sizeof(ivec4) * boneWeights.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	VulkanAllocatedMemory weightsStagingBuffer;
	weightsStagingBuffer = WillEngine::VulkanUtil::createBuffer(vmaAllocator, sizeof(vec4) * boneWeights.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	VulkanAllocatedMemory indexStagingBuffer;
	indexStagingBuffer = WillEngine::VulkanUtil::createBuffer(vmaAllocator, sizeof(u32) * indicies.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	// Copy data to the staging buffer first
	void* positionPtr = nullptr;
	if (vmaMapMemory(vmaAllocator, positionStagingBuffer.allocation, &positionPtr) != VK_SUCCESS)
		throw std::runtime_error("Failed to map memory");
	std::memcpy(positionPtr, positions.data(), sizeof(vec3) * positions.size());
	vmaUnmapMemory(vmaAllocator, positionStagingBuffer.allocation);

	void* normalPtr = nullptr;
	if (vmaMapMemory(vmaAllocator, normalStagingBuffer.allocation, &normalPtr) != VK_SUCCESS)
		throw std::runtime_error("Failed to map memory");
	std::memcpy(normalPtr, normals.data(), sizeof(vec3) * normals.size());
	vmaUnmapMemory(vmaAllocator, normalStagingBuffer.allocation);

	void* tangentPtr = nullptr;
	if (vmaMapMemory(vmaAllocator, tangentStagingBuffer.allocation, &tangentPtr) != VK_SUCCESS)
		throw std::runtime_error("Failed to map memory");
	std::memcpy(tangentPtr, tangents.data(), sizeof(vec3) * tangents.size());
	vmaUnmapMemory(vmaAllocator, tangentStagingBuffer.allocation);

	void* uvPtr = nullptr;
	if (vmaMapMemory(vmaAllocator, uvStagingBuffer.allocation, &uvPtr) != VK_SUCCESS)
		throw std::runtime_error("Failed to map memory");
	std::memcpy(uvPtr, uvs.data(), sizeof(vec2) * uvs.size());
	vmaUnmapMemory(vmaAllocator, uvStagingBuffer.allocation);

	void* boneIdsPtr = nullptr;
	if (vmaMapMemory(vmaAllocator, boneIdsStagingBuffer.allocation, &boneIdsPtr) != VK_SUCCESS)
		throw std::runtime_error("Failed to map memory");
	std::vector<ivec4> boneIdsData;
	for (u32 i = 0; i < boneWeights.size(); i++)
	{
		ivec4 boneIds(-1);
		boneIds.x = boneWeights[i].boneIds[0];
		boneIds.y = boneWeights[i].boneIds[1];
		boneIds.z = boneWeights[i].boneIds[2];
		boneIds.w = boneWeights[i].boneIds[3];

		boneIdsData.push_back(boneIds);
	}
	std::memcpy(boneIdsPtr, boneIdsData.data(), sizeof(ivec4) * boneWeights.size());
	vmaUnmapMemory(vmaAllocator, boneIdsStagingBuffer.allocation);

	void* weightsPtr = nullptr;
	if (vmaMapMemory(vmaAllocator, weightsStagingBuffer.allocation, &weightsPtr) != VK_SUCCESS)
		throw std::runtime_error("Failed to map memory");
	std::vector<vec4> weightsData;
	for (u32 i = 0; i < boneWeights.size(); i++)
	{
		vec4 weights(0);
		weights.x = boneWeights[i].weights[0];
		weights.y = boneWeights[i].weights[1];
		weights.z = boneWeights[i].weights[2];
		weights.w = boneWeights[i].weights[3];

		weightsData.push_back(weights);
	}
	std::memcpy(weightsPtr, weightsData.data(), sizeof(vec4) * boneWeights.size());
	vmaUnmapMemory(vmaAllocator, weightsStagingBuffer.allocation);

	void* indexPtr = nullptr;
	if (vmaMapMemory(vmaAllocator, indexStagingBuffer.allocation, &indexPtr) != VK_SUCCESS)
		throw std::runtime_error("Failed to map memory");
	std::memcpy(indexPtr, indicies.data(), sizeof(u32) * indicies.size());
	vmaUnmapMemory(vmaAllocator, indexStagingBuffer.allocation);

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
	vkCmdCopyBuffer(commandBuffer, positionStagingBuffer.buffer, positionBuffer.buffer, 1, &positionCopy);

	WillEngine::VulkanUtil::bufferBarrier(commandBuffer, positionBuffer.buffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_WHOLE_SIZE, 0, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);

	VkBufferCopy normalCopy{};
	normalCopy.size = sizeof(vec3) * normals.size();
	vkCmdCopyBuffer(commandBuffer, normalStagingBuffer.buffer, normalBuffer.buffer, 1, &normalCopy);

	WillEngine::VulkanUtil::bufferBarrier(commandBuffer, normalBuffer.buffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_WHOLE_SIZE, 0, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);

	VkBufferCopy tangentCopy{};
	tangentCopy.size = sizeof(vec3) * tangents.size();
	vkCmdCopyBuffer(commandBuffer, tangentStagingBuffer.buffer, tangentBuffer.buffer, 1, &tangentCopy);

	WillEngine::VulkanUtil::bufferBarrier(commandBuffer, tangentBuffer.buffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_WHOLE_SIZE, 0, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);

	VkBufferCopy uvCopy{};
	uvCopy.size = sizeof(vec2) * uvs.size();
	vkCmdCopyBuffer(commandBuffer, uvStagingBuffer.buffer, uvBuffer.buffer, 1, &uvCopy);

	WillEngine::VulkanUtil::bufferBarrier(commandBuffer, uvBuffer.buffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_WHOLE_SIZE, 0, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);

	VkBufferCopy boneIdsCopy{};
	boneIdsCopy.size = sizeof(ivec4) * boneWeights.size();
	vkCmdCopyBuffer(commandBuffer, boneIdsStagingBuffer.buffer, boneIdsBuffer.buffer, 1, &boneIdsCopy);

	WillEngine::VulkanUtil::bufferBarrier(commandBuffer, boneIdsBuffer.buffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_WHOLE_SIZE, 0, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);

	VkBufferCopy weightsCopy{};
	weightsCopy.size = sizeof(vec4) * boneWeights.size();
	vkCmdCopyBuffer(commandBuffer, weightsStagingBuffer.buffer, weightsBuffer.buffer, 1, &weightsCopy);

	WillEngine::VulkanUtil::bufferBarrier(commandBuffer, weightsBuffer.buffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_WHOLE_SIZE, 0, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);

	VkBufferCopy indexCopy{};
	indexCopy.size = sizeof(u32) * indicies.size();
	vkCmdCopyBuffer(commandBuffer, indexStagingBuffer.buffer, indexBuffer.buffer, 1, &indexCopy);

	WillEngine::VulkanUtil::bufferBarrier(commandBuffer, indexBuffer.buffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
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

	readyToDraw = true;

	// Clean up staging buffers
	vmaDestroyBuffer(vmaAllocator, positionStagingBuffer.buffer, positionStagingBuffer.allocation);
	vmaDestroyBuffer(vmaAllocator, normalStagingBuffer.buffer, normalStagingBuffer.allocation);
	vmaDestroyBuffer(vmaAllocator, tangentStagingBuffer.buffer, tangentStagingBuffer.allocation);
	vmaDestroyBuffer(vmaAllocator, uvStagingBuffer.buffer, uvStagingBuffer.allocation);
	vmaDestroyBuffer(vmaAllocator, boneIdsStagingBuffer.buffer, boneIdsStagingBuffer.allocation);
	vmaDestroyBuffer(vmaAllocator, weightsStagingBuffer.buffer, weightsStagingBuffer.allocation);
	vmaDestroyBuffer(vmaAllocator, indexStagingBuffer.buffer, indexStagingBuffer.allocation);

	// Clean up command pool, command buffer, and fence
	vkDestroyFence(logicalDevice, uploadComplete, nullptr);
	vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
	vkDestroyCommandPool(logicalDevice, commandPool, nullptr);

	// Remove unnecessary data from memory
	positions.clear();
	normals.clear();
	tangents.clear();
	uvs.clear();
	indicies.clear();

	positions.shrink_to_fit();
	normals.shrink_to_fit();
	tangents.clear();
	uvs.shrink_to_fit();
	indicies.shrink_to_fit();
}

std::vector<VkBuffer> SkinnedMesh::getVulkanBuffers() const
{
	std::vector<VkBuffer> returnBuffers = { positionBuffer.buffer, normalBuffer.buffer, tangentBuffer.buffer, uvBuffer.buffer, 
		boneIdsBuffer.buffer, weightsBuffer.buffer };

	return std::move(returnBuffers);
}

std::vector<VkDeviceSize> SkinnedMesh::getVulkanOffset() const
{
	std::vector<VkDeviceSize> returnOffsets(getVulkanBufferSize());

	return std::move(returnOffsets);
}