#pragma once
#include <queue>

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

	public:

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

		bool readyToDraw;

	public:

		MeshComponent();
		MeshComponent(Entity* entity);
		MeshComponent(Entity* entity, const MeshComponent* mesh);
		virtual ~MeshComponent();

		virtual void update() {};

		void uploadDataToPhysicalDevice(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkSurfaceKHR& surface, VkQueue& queue);

		virtual ComponentType getType() { return id; };

		void cleanup(VkDevice& logicalDevice, VmaAllocator vmaAllocator);

		bool isReadyToDraw() const { return readyToDraw; };
	};
}
