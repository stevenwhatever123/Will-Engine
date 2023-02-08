#pragma once
#include <typeindex>

namespace WillEngine
{
	enum ComponentType : u32
	{
		NullType = 0,
		TransformType,
		MeshType,
		SkinnedMeshType,
		LightType,
		SkeletalType,
		ComponentTypeCount,
		// Any Type below here is private and should not be visible to the user
		IRenderableType
	};

	extern std::map<std::type_index, ComponentType> componentTypeMap;
	extern std::map<ComponentType, std::string> componentTypeName;

	void initComponentType();
}