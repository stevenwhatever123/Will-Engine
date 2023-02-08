#pragma once
#include "Core/ECS/IRenderableComponent.h"
#include "Core/Mesh.h"
#include "Core/Material.h"

namespace WillEngine
{
	class SkinnedMeshComponent : public IRenderableComponent
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
		virtual u32 getNumMesh() const { return meshIndicies.size(); };

		virtual void update() {};

		virtual ComponentType getType() { return id; };

	};
}
