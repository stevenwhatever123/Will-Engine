#pragma once
#include "Core/BoneWeight.h"

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
	std::vector<vec3> tangents;
	std::vector<vec3> bitangents;
	std::vector<vec2> uvs;
	std::vector<u32> indicies;

public:

	u32 indiciesSize;

	// Vertex Buffer
	VulkanAllocatedMemory positionBuffer;
	VulkanAllocatedMemory normalBuffer;
	VulkanAllocatedMemory tangentBuffer;
	VulkanAllocatedMemory bitangentBuffer;
	VulkanAllocatedMemory uvBuffer;
	VulkanAllocatedMemory indexBuffer;

	VkPrimitiveTopology primitive;

private:

	// Used for generating an id for a mesh
	static u32 idCounter;

protected:

	bool readyToDraw;

public:

	Mesh();
	Mesh(const Mesh* mesh);
	virtual ~Mesh();

	virtual void uploadDataToPhysicalDevice(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkSurfaceKHR& surface, VkQueue& queue);

	virtual std::vector<VkBuffer> getVulkanBuffers() const;
	virtual u32 getVulkanBufferSize() const { return 5; };
	virtual std::vector<VkDeviceSize> getVulkanOffset() const;

	virtual void cleanup(VkDevice& logicalDevice, VmaAllocator vmaAllocator);

	virtual bool isReadyToDraw() const { return readyToDraw; };
};