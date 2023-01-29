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

TransformComponent::TransformComponent(const vec3 position, const vec3 rotation, const vec3 scale) :
	Component(nullptr),
	position(position),
	rotation(rotation),
	scale(scale)
{

}

TransformComponent::TransformComponent(Entity* entity) :
	Component(entity),
	position(0),
	rotation(0),
	scale(1)
{

}

TransformComponent::TransformComponent(Entity* entity, const vec3 position, const vec3 rotation, const vec3 scale) :
	Component(entity),
	position(position),
	rotation(rotation),
	scale(scale)
{

}

TransformComponent::~TransformComponent()
{

}

mat4 TransformComponent::getLocalTransformation() const
{
	//// Translate
	//mat4 translation = glm::translate(mat4(1), position);

	//// Rotation
	//mat4 rotate = glm::eulerAngleXYZ(rotation.x, rotation.y, rotation.z);

	//// Scale
	//mat4 scaling = glm::scale(mat4(1), scale);

	//return translation * rotate * scaling;


	// Scale
	mat4 scaling = glm::scale(mat4(1), scale);

	// Rotation
	mat4 rotate = glm::eulerAngleXYZ(rotation.x, rotation.y, rotation.z);

	// Translate
	return  glm::translate(rotate * scaling, position);
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