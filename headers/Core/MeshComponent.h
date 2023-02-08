#pragma once
#include "Utils/VulkanUtil.h"

#include "Core/ECS/Component.h"
#include "Core/Mesh.h"
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
		std::vector<u32> meshIndicies;
		std::vector<u32> materialIndicies;

	public:

		MeshComponent();
		MeshComponent(const MeshComponent* meshComp);
		virtual ~MeshComponent();

		void addMesh(Mesh* mesh);
		void addMesh(Mesh* mesh, Material* material);

		virtual void update() {};

		virtual u32 getNumMesh() const { return meshIndicies.size(); };

		virtual ComponentType getType() { return id; };
	};
}
