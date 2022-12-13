#pragma once
#include "Utils/VulkanUtil.h"

#include "ECS/Component.h"

#include "Core/Material.h"
#include "Core/UniformClass.h"

#include "Managers/FileManager.h"

namespace WillEngine
{
	class MeshComponent : public Component
	{
	public:

		static const ComponentType id = ComponentType::MeshType;

		std::string name;
		u32 materialIndex;

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

		MeshComponent();
		virtual ~MeshComponent();

		void uploadDataToPhysicalDevice(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkSurfaceKHR& surface, VkQueue& queue);

		void updateForPushConstant();

		void cleanup(VkDevice& logicalDevice, VmaAllocator vmaAllocator);
	};
}
