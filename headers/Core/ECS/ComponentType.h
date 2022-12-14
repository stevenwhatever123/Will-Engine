#pragma once
#include <typeindex>

namespace WillEngine
{
	enum ComponentType : u32
	{
		AbstractType = 0,
		TransformType,
		MeshType,
		LightType
	};

	extern std::map<std::type_index, ComponentType> componentTypeMap;
	extern std::map<ComponentType, std::string> componentTypeName;

	void initComponentType();
}