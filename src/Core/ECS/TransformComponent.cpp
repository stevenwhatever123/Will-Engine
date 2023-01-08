#include "pch.h"
#include "Core/ECS/TransformComponent.h"

using namespace WillEngine;

TransformComponent::TransformComponent():
	Component(nullptr),
	position(0),
	rotation(0),
	scale(1)
{

}

TransformComponent::TransformComponent(Entity* entity) :
	Component(entity),
	position(0),
	rotation(0),
	scale(1)
{

}

TransformComponent::~TransformComponent()
{

}

mat4 TransformComponent::getLocalTransformation() const
{
	// Translate
	mat4 translation = glm::translate(mat4(1), position);

	// Rotation
	//mat4 rotation = glm::rotate(mat4(1), rotation);

	// Scale
	mat4 scaling = glm::scale(mat4(1), scale);

	return scaling * translation;
}

mat4 TransformComponent::getGlobalTransformation() const
{
	mat4 resultMatrix = getLocalTransformation();

	Entity* parentEntity = parent->parent;
	while (parentEntity)
	{
		TransformComponent* transformComp = parentEntity->GetComponent<TransformComponent>();
		resultMatrix = transformComp->getLocalTransformation() * resultMatrix;

		parentEntity = parentEntity->parent;
	}

	return resultMatrix;
}