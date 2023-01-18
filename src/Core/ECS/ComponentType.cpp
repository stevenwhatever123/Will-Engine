#include "pch.h"
#include "Core/ECS/ComponentType.h"

#include "Core/ECS/Component.h"
#include "Core/ECS/TransformComponent.h"
#include "Core/MeshComponent.h"
#include "Core/ECS/SkinnedMeshComponent.h"
#include "Core/ECS/SkeletalComponent.h"
#include "Core/LightComponent.h"

namespace WillEngine
{
	// Defining global variable
	std::map<std::type_index, ComponentType> componentTypeMap;
	std::map<ComponentType, std::string> componentTypeName;
}

void WillEngine::initComponentType()
{
	componentTypeMap =
	{
		{typeid(Component),									NullType},
		{typeid(TransformComponent),						TransformType},
		{typeid(MeshComponent),								MeshType},
		{typeid(SkinnedMeshComponent),						SkinnedMeshType},
		{typeid(SkeletalComponent),							SkeletalType},
		{typeid(LightComponent),							LightType}
	};

	componentTypeName =
	{
		{NullType,											"Null Component"},
		{TransformType,										"Transform Component"},
		{MeshType,											"Mesh Component"},
		{SkinnedMeshType,									"Skinned Mesh Component"},
		{SkeletalType,										"Skeletal Component"},
		{LightType,											"Light Component"}
	};
}