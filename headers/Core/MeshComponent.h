#pragma once
#include "Utils/VulkanUtil.h"

#include "Core/ECS/IRenderableComponent.h"
#include "Core/Mesh.h"
#include "Core/Material.h"
#include "Core/UniformClass.h"

#include "Managers/FileManager.h"

namespace WillEngine
{
	class MeshComponent : public IRenderableComponent
	{
	public:

		static const ComponentType id = ComponentType::MeshType;

	public:

		std::string name;
		u32 meshIndex;
		u32 materialIndex;

	public:

		MeshComponent();
		MeshComponent(const Mesh* mesh);
		virtual ~MeshComponent();

		void setMesh(Mesh* mesh);

		virtual void update() {};

		virtual u32 getNumMesh() const { return 1; };

		virtual ComponentType getType() { return id; };
	};
}
