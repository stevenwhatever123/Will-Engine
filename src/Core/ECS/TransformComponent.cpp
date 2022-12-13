#include "pch.h"
#include "Core/ECS/TransformComponent.h"

using namespace WillEngine;

TransformComponent::TransformComponent():
	position(0),
	rotation(0),
	scale(1)
{

}

TransformComponent::~TransformComponent()
{

}

void TransformComponent::updateModelTransformation()
{
	mat4 initialMatrix(1);

	// Scale
	modelTransformation = glm::scale(initialMatrix, scale);

	// Rotation
	//modelTransformation = glm::rotate(modelTransformation, rotation);

	// Translate
	modelTransformation = glm::translate(modelTransformation, position);
}