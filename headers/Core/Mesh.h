#pragma once
#include "Utils/VulkanUtil.h"

#include "Core/Material.h"

#include "Managers/FileManager.h"

class Mesh
{
public:

	std::string name;
	u32 materialIndex;

	std::vector<vec3> positions;
	std::vector<vec3> normals;
	std::vector<vec2> uvs;
	std::vector<u32> indicies;

public:

	u32 indiciesSize;

	VkShaderModule vertShader;
	VkShaderModule fragShader;

	// Vertex Buffer
	VulkanAllocatedMemory positionBuffer;
	VulkanAllocatedMemory normalBuffer;
	VulkanAllocatedMemory uvBuffer;
	VulkanAllocatedMemory indexBuffer;

	VkPrimitiveTopology primitive;

	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

public:

	Mesh();
	~Mesh();

	void uploadDataToPhysicalDevice(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkSurfaceKHR& surface, VkQueue& queue);

	void initShaderModules(VkDevice& logicalDevice);

	void dataUploaded();

	void cleanup(VkDevice& logicalDevice, VmaAllocator vmaAllocator);
};
