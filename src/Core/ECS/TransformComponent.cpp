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

mat4 TransformComponent::getAnimationLocalTransformation(const Animation* animation, const AnimationComponent* animationComp) const
{
	Entity* currentEntity = parent;

	vec3 animationPosition(0);
	quat animationRotation(0, 0, 0, 0);
	vec3 animationScale(0);
	for (u32 i = 0; i < animation->getNumAnimationNode(); i++)
	{
		const AnimationNode& animationNode = animation->animationNodes[i];

		if (currentEntity->name == animationNode.name)
		{
			u32 positionKey = animationComp->getPositionKeyIndex(i);
			animationPosition = animationNode.getPosition(positionKey).value;

			u32 rotationKey = animationComp->getRotationKeyIndex(i);
			animationRotation = animationNode.getRotation(rotationKey).value;

			u32 scaleKey = animationComp->getScaleKeyIndex(i);
			animationScale = animationNode.getScale(scaleKey).value;

			// Translate
			mat4 translation = glm::translate(mat4(1), animationPosition);

			// Rotation
			mat4 rotate = glm::mat4(animationRotation);

			// Scaling
			return glm::scale(translation * rotate, animationScale);
		}
	}

	return getLocalTransformation();
}

mat4 TransformComponent::getAnimationGlobalTransformation(const Animation* animation, const AnimationComponent* animationComp) const
{
	mat4 resultMatrix = getAnimationLocalTransformation(animation, animationComp);

	if (!parent->hasParent())
		return resultMatrix;

	Entity* parentEntity = parent->getParent();
	while (parentEntity)
	{
		TransformComponent* transformComp = parentEntity->GetComponent<TransformComponent>();
		resultMatrix = transformComp->getAnimationLocalTransformation(animation, animationComp) * resultMatrix;

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

void TransformComponent::updateAllChildAnimationWorldTransformation(const Animation* animation, const AnimationComponent* animationComp)
{
	updateAnimationWorldTransformation(animation, animationComp);

	for (auto child : getParent()->children)
	{
		TransformComponent* childTransform = child->GetComponent<TransformComponent>();
		childTransform->updateAllChildAnimationWorldTransformation(animation, animationComp);
	}
}