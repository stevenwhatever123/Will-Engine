#pragma once
#include "Utils/VulkanUtil.h"

#include "Managers/FileManager.h"

class Mesh
{
public:

	// unique id for mesh
	const u32 id;
	std::string name;
	u32 materialIndex;

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

private:

	// Used for generating an id for a mesh
	static u32 idCounter;

	bool readyToDraw;

public:

	Mesh();
	Mesh(const Mesh* mesh);
	virtual ~Mesh();

	void uploadDataToPhysicalDevice(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkSurfaceKHR& surface, VkQueue& queue);

	void cleanup(VkDevice& logicalDevice, VmaAllocator vmaAllocator);

	bool isReadyToDraw() const { return readyToDraw; };
};