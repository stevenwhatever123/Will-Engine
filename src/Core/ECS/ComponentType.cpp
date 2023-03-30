#include "pch.h"
#include "Core/ECS/ComponentType.h"

#include "Core/ECS/Component.h"
#include "Core/ECS/TransformComponent.h"
#include "Core/MeshComponent.h"
#include "Core/ECS/SkeletalComponent.h"
#include "Core/LightComponent.h"
#include "Core/ECS/AnimationComponent.h"

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
		{typeid(LightComponent),							LightType},
		{typeid(SkeletalComponent),							SkeletalType},
		{typeid(AnimationComponent),						AnimationType},			
		// Any Type below here is private and should not be visible to the user
	};

	componentTypeName =
	{
		{NullType,											"Null Component"},
		{TransformType,										"Transform Component"},
		{MeshType,											"Mesh Component"},
		{LightType,											"Light Component"},
		{SkeletalType,										"Skeletal Component"},
		{AnimationType,										"Animation Component"},
		// Any Type below here is private and should not be visible to the user
	};
}