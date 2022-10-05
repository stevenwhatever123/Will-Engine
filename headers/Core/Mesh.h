#pragma once
#include "Utils/VulkanUtil.h"

#include "Core/Material.h"

#include "Managers/FileManager.h"

class Mesh
{
public:

	std::string name;
	u32 materialIndex;

	vec3 transformPosition;
	mat4 modelMatrix;

	std::vector<vec3> positions;
	std::vector<vec3> normals;
	std::vector<vec2> uvs;
	std::vector<u32> indicies;

public:

	u32 indiciesSize;

	// Vertex Buffer
	VulkanAllocatedMemory positionBuffer;
	VulkanAllocatedMemory normalBuffer;
	VulkanAllocatedMemory uvBuffer;
	VulkanAllocatedMemory indexBuffer;

	VkPrimitiveTopology primitive;

public:

	Mesh();
	~Mesh();

	void uploadDataToPhysicalDevice(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkSurfaceKHR& surface, VkQueue& queue);

	void dataUploaded();

	void updateModelMatrix();

	void cleanup(VkDevice& logicalDevice, VmaAllocator vmaAllocator);
};
