#include "pch.h"
#include "Core/ECS/Entity.h"

// Components
#include "Core/ECS/TransformComponent.h"
#include "Core/MeshComponent.h"

using namespace WillEngine;

Entity::Entity() :
	isEnable(true),
	name(""),
	components()
{

}

Entity::Entity(const char* name) :
	isEnable(true),
	name(name),
	components()
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

// Explicit initialization for addComponent
template void Entity::addComponent(TransformComponent* comp);
template void Entity::addComponent(MeshComponent* comp);