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

void TransformComponent::update()
{
	mat4 initialMatrix(1);

	// Translate
	modelTransformation = glm::translate(initialMatrix, position);

	// Rotation
	//modelTransformation = glm::rotate(modelTransformation, rotation);

	// Scale
	modelTransformation = glm::scale(modelTransformation, scale);
}