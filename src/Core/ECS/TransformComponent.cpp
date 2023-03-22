#include "pch.h"
#include "Core/ECS/TransformComponent.h"

using namespace WillEngine;

TransformComponent::TransformComponent():
	Component(nullptr),
	position(0),
	rotation(0),
	scale(1),
	worldTransformation(1)
{
	
}

TransformComponent::TransformComponent(const vec3 position, const vec3 rotation, const vec3 scale) :
	Component(nullptr),
	position(position),
	rotation(rotation),
	scale(scale),
	worldTransformation(1)
{
	
}

TransformComponent::TransformComponent(Entity* entity) :
	Component(entity),
	position(0),
	rotation(0),
	scale(1)
{
	worldTransformation = getGlobalTransformation();
}

TransformComponent::TransformComponent(Entity* entity, const vec3 position, const vec3 rotation, const vec3 scale) :
	Component(entity),
	position(position),
	rotation(rotation),
	scale(scale)
{
	worldTransformation = getGlobalTransformation();
}

TransformComponent::~TransformComponent()
{
	
}

mat4 TransformComponent::getLocalTransformation() const
{
	// Translate
	mat4 translation = glm::translate(mat4(1), position);

	// Rotation
	mat4 rotate = glm::eulerAngleXYZ(rotation.x, rotation.y, rotation.z);

	// Scaling
	return glm::scale(translation * rotate, scale);
}

mat4 TransformComponent::getGlobalTransformation() const
{
	mat4 resultMatrix = getLocalTransformation();

	if (!parent->hasParent())
		return resultMatrix;

	Entity* parentEntity = parent->getParent();
	while (parentEntity)
	{
		TransformComponent* transformComp = parentEntity->GetComponent<TransformComponent>();
		resultMatrix = transformComp->getLocalTransformation() * resultMatrix;

		parentEntity = parentEntity->getParent();
	}

	return resultMatrix;
}

void TransformComponent::updateAllChildWorldTransformation()
{
	updateWorldTransformation();

	for (auto child : getParent()->children)
	{
		TransformComponent* childTransform = child->GetComponent<TransformComponent>();
		childTransform->updateAllChildWorldTransformation();
	}
}