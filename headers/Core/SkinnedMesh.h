#pragma once
#include "Core/Mesh.h"

class SkinnedMesh : public Mesh
{
public:

	std::vector<BoneWeight> boneWeights;

public:

	VulkanAllocatedMemory boneIdsBuffer;
	VulkanAllocatedMemory weightsBuffer;

public:
	
	SkinnedMesh();
	virtual ~SkinnedMesh();

	virtual void uploadDataToPhysicalDevice(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkSurfaceKHR& surface, VkQueue& queue);
};