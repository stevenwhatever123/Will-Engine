#pragma once
#include "Core/Mesh.h"
#include "Core/Material.h"

#include "Core/ECS/Component.h"

namespace WillEngine
{
	class SkinnedMeshComponent : public Component
	{
	public:

		static const ComponentType id = ComponentType::SkinnedMeshType;

	public:

		std::string name;
		std::vector<u32> meshIndicies;
		std::vector<u32> materialIndicies;

	public:

		SkinnedMeshComponent();
		SkinnedMeshComponent(const SkinnedMeshComponent* skinnedMeshComp);
		virtual ~SkinnedMeshComponent();

		void addMesh(Mesh* mesh, Material* material);

		virtual void update() {};

		virtual ComponentType getType() { return id; };

	};
}
