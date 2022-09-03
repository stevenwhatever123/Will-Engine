#pragma once
#include "Utils/VulkanUtil.h"

#include "Managers/FileManager.h"

class Mesh
{
private:

	VkShaderModule vertShader;
	VkShaderModule fragShader;

public:

	std::string name;

	std::vector<vec3> positions;
	std::vector<vec3> normals;
	std::vector<vec2> uvs;
	std::vector<u32> indicies;

	// Vertex Buffer
	VkBuffer vertexBuffer;
	VmaAllocation vertexAllocation;

	VkBuffer normalBuffer;
	VmaAllocation normalAllocation;

	VkBuffer uvBuffer;
	VmaAllocation uvAllocation;

	VkBuffer indexBuffer;
	VmaAllocation indexAllocation;

	// Uniform buffer
	mat4 transformation;
	VkBuffer buffer;
	VmaAllocation vmaAllocation;

	VkPrimitiveTopology primitive;

	VkDescriptorSet descriptorSet;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

public:

	Mesh();
	~Mesh();

	void sendDataToGPU(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkSurfaceKHR& surface, VkQueue& queue);

	void generatePipelineLayout(VkDevice& logicalDevice, VkDescriptorSetLayout& descriptorSetLayout);

	void generatePipeline(VkDevice& logicalDevice, VkRenderPass& renderpass, VkExtent2D swapchainExtent);

	void cleanup(VkDevice& logicalDevice);
};
