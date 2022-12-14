#include "pch.h"
#include "Core/ECS/Entity.h"

// Components
#include "Core/ECS/TransformComponent.h"
#include "Core/MeshComponent.h"
#include "Core/ECS/SkinnedMeshComponent.h"
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

Entity::Entity(Entity* parent, const char* name):
	isEnable(true),
	id(++idCounter),
	name(name),
	components(),
	parent(parent),
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

void Entity::addChild(Entity* child)
{
	this->children.push_back(child);
}

template<typename T> 
void Entity::addComponent(T* comp)
{
	if (comp->id == ComponentType::NullType)
		throw std::runtime_error("Component must not be a null type");

	// Don't add this component if it is not a part of our pre-defined one
	if (componentTypeMap.find(typeid(T)) == componentTypeMap.end())
		return;

	// Don't add this component if the same type already exist
	if (HasComponent<T>())
		return;

	components[typeid(T)] = comp;
	components[typeid(T)]->setParent(this);
}

// Explicit initialization for addComponent(T* comp)
template void Entity::addComponent(TransformComponent* comp);
template void Entity::addComponent(MeshComponent* comp);
template void Entity::addComponent(SkinnedMeshComponent* comp);
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
template void Entity::addComponent<SkinnedMeshComponent>();
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
			throw std::runtime_error("You cannot add a Mesh Type here.....");
			break;
		}
		case SkinnedMeshType:
		{
			throw std::runtime_error("You cannot add a Mesh Type here.....");
			break;
		}
		case LightType:
		{
			LightComponent* light = new LightComponent();
			components[typeid(LightComponent)] = light;
			break;
		}
	}
}