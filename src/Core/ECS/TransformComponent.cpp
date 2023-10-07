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

mat4 TransformComponent::getLocalTransformation(const Animation* animation, const AnimationComponent* animationComp) const
{
	Entity* currentEntity = parent;

	if (animation->animationNodes.contains(currentEntity->name))
	{
		const AnimationNode& animationNode = animation->animationNodes.find(currentEntity->name)->second;

		u32 positionKey = animationComp->getPositionKeyIndex(currentEntity->name);
		const vec3& animationPosition = animationNode.getPosition(positionKey).value;

		u32 rotationKey = animationComp->getRotationKeyIndex(currentEntity->name);
		const quat& animationRotation = animationNode.getRotation(rotationKey).value;

		u32 scaleKey = animationComp->getScaleKeyIndex(currentEntity->name);
		const vec3& animationScale = animationNode.getScale(scaleKey).value;

		// Translate
		mat4 translation = glm::translate(mat4(1), animationPosition);

		// Rotation
		mat4 rotate = glm::mat4(animationRotation);

		// Scaling
		return glm::scale(translation * rotate, animationScale);
	}

	return getLocalTransformation();
}

mat4 TransformComponent::getGlobalTransformation(const Animation* animation, const AnimationComponent* animationComp) const
{
	mat4 resultMatrix = getLocalTransformation(animation, animationComp);

	if (!parent->hasParent())
		return resultMatrix;

	Entity* parentEntity = parent->getParent();

	if (parentEntity)
	{
		TransformComponent* transformComp = parentEntity->GetComponent<TransformComponent>();
		resultMatrix = transformComp->getWorldTransformation() * resultMatrix;
	}

	return resultMatrix;
}

void TransformComponent::updateAllChildWorldTransformation()
{
	updateWorldTransformation();

	for (auto* child : getParent()->children)
	{
		TransformComponent* childTransform = child->GetComponent<TransformComponent>();
		childTransform->updateAllChildWorldTransformation();
	}
}

void TransformComponent::updateAllChildWorldTransformation(const Animation* animation, const AnimationComponent* animationComp, const std::unordered_map<std::string, bool>* necessityMap)
{
	// Don't update transformation if all of the following is met:
	// 1. We have a necessity map included
	// 2. It does not appear in the necessity map / The node appears in the necessity map but has not been visited before
	// More details (Bone Section): https://assimp.sourceforge.net/lib_html/data.html
	if (necessityMap && (!necessityMap->contains(parent->name) || necessityMap->at(parent->name) == false))
		return;

	updateWorldTransformation(animation, animationComp);

	for (auto* child : getParent()->children)
	{
		TransformComponent* childTransform = child->GetComponent<TransformComponent>();
		childTransform->updateAllChildWorldTransformation(animation, animationComp, necessityMap);
	}
}