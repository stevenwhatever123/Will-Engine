#include "pch.h"
#include "Core/ECS/Entity.h"

// Components
#include "Core/ECS/TransformComponent.h"
#include "Core/MeshComponent.h"
#include "Core/LightComponent.h"

#include "Utils/ModelImporter.h"

using namespace WillEngine;

u32 Entity::idCounter = 0;

Entity::Entity() :
	isEnable(true),
	id(++idCounter),
	name(""),
	components(),
	parent(nullptr),
	children()
{

}

Entity::Entity(const char* name) :
	isEnable(true),
	id(++idCounter),
	name(name),
	components(),
	parent(nullptr),
	children()
{

}

Entity::~Entity()
{

}

void Entity::setName(const char* name)
{
	this->name = name;
}

template<typename T> 
void Entity::addComponent(T* comp)
{
	if (comp->id == ComponentType::AbstractType)
		throw std::runtime_error("Component must not be an abstract type");

	// Don't add this component if it is not a part of our pre-defined one
	if (componentTypeMap.find(typeid(T)) == componentTypeMap.end())
		return;

	// Don't add this component if the same type already exist
	if (components.find(typeid(T)) != components.end())
		return;

	components[typeid(T)] = comp;
}

// Explicit initialization for addComponent(T* comp)
template void Entity::addComponent(TransformComponent* comp);
template void Entity::addComponent(MeshComponent* comp);
template void Entity::addComponent(LightComponent* comp);

template<class T>
void Entity::addComponent()
{
	T* comp = new T();

	components[typeid(T)] = comp;
}

// Explicit initialization for addComponent<T>()
template void Entity::addComponent<TransformComponent>();
template void Entity::addComponent<MeshComponent>();
template void Entity::addComponent<LightComponent>();

void Entity::addComponent(ComponentType type)
{
	switch (type)
	{
		case TransformType:
		{
			TransformComponent* transform = new TransformComponent(this);
			components[typeid(TransformComponent)] = transform;
			break;
		}
		case MeshType:
		{
			std::vector<MeshComponent*> meshes;
			std::vector<Material*> materials;

			std::string defaultPreset = "C:/Users/Steven/source/repos/Will-Engine/presets/meshes/cube.fbx";
			std::tie(meshes, materials) = WillEngine::Utils::readModel(defaultPreset.c_str());

			MeshComponent* mesh = new MeshComponent(this, meshes[0]);
			components[typeid(MeshComponent)] = mesh;

			for (auto mesh : meshes)
				delete mesh;

			for (auto material : materials)
				delete material;

			break;
		}
		case LightType:
		{
			LightComponent* light = new LightComponent(this);
			components[typeid(LightComponent)] = light;
			break;
		}
	}
}