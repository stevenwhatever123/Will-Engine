#pragma once
#include "Utils/VulkanUtil.h"

#include "ECS/Component.h"

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
		u32 meshIndex;
		u32 materialIndex;

	public:

		MeshComponent();
		MeshComponent(const Mesh* mesh);
		virtual ~MeshComponent();

		void setMesh(Mesh* mesh);

		virtual void update() {};

		virtual ComponentType getType() { return id; };

	};
}
