#include "pch.h"
#include "Core/ECS/ComponentType.h"

#include "Core/ECS/Component.h"
#include "Core/MeshComponent.h"
#include "Core/ECS/TransformComponent.h"

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
		{typeid(Component),									AbstractType},
		{typeid(TransformComponent),						TransformType},
		{typeid(MeshComponent),										MeshType}
	};

	componentTypeName =
	{
		{AbstractType,										"Abstract Component"},
		{TransformType,										"Transform Component"},
		{MeshType,											"Mesh Component"}
	};
}