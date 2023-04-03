#include "pch.h"
#include "Core/ECS/AnimationComponent.h"

using namespace WillEngine;

AnimationComponent::AnimationComponent():
	Component(nullptr),
	playAnimation(false),
	selectedAnimationIndex(0),
	animationIds(),
	time(0),
	accumulator(0)
{

}

AnimationComponent::AnimationComponent(Entity* entity) :
	Component(entity),
	playAnimation(false),
	selectedAnimationIndex(0),
	animationIds(),
	time(0),
	accumulator(0)
{

}

AnimationComponent::~AnimationComponent()
{

}

void AnimationComponent::addAnimation(Animation* animation)
{
	animationIds.push_back(animation->id);

	std::unordered_map<std::string, u32> animationNodePositionIndex;
	std::unordered_map<std::string, u32> animationNodeRotationIndex;
	std::unordered_map<std::string, u32> animationNodeScaleIndex;

	for (auto animationNode : animation->animationNodes)
	{
		animationNodePositionIndex[animationNode.first] = 0;
		animationNodeRotationIndex[animationNode.first] = 0;
		animationNodeScaleIndex[animationNode.first] = 0;
	}

	positionIndicies.push_back(animationNodePositionIndex);
	rotationIndicies.push_back(animationNodeRotationIndex);
	scaleIndicies.push_back(animationNodeScaleIndex);
}

u32 AnimationComponent::getPositionIndex(const Animation* animation, const AnimationNode* animationNode, u32 currentIndex) const
{
	f64 ticksPerSecond = animation->getTicksPerSecond();

	for (u32 i = currentIndex; i < animationNode->getNumPosition(); i++)
	{
		if (time < animationNode->positions[i].time / ticksPerSecond)
			return i;
	}

	// Return the last index if not found as it usually means we have reach the end
	return animationNode->getNumPosition() - 1;
}

u32 AnimationComponent::getRotationIndex(const Animation* animation, const AnimationNode* animationNode, u32 currentIndex) const
{
	f64 ticksPerSecond = animation->getTicksPerSecond();

	for (u32 i = currentIndex; i < animationNode->getNumRotation(); i++)
	{
		if (time < animationNode->rotations[i].time / ticksPerSecond)
			return i;
	}

	// Return the last index if not found as it usually means we have reach the end
	return animationNode->getNumRotation() - 1;
}

u32 AnimationComponent::getScaleIndex(const Animation* animation, const AnimationNode* animationNode, u32 currentIndex) const
{
	f64 ticksPerSecond = animation->getTicksPerSecond();

	for (u32 i = currentIndex; i < animationNode->getNumScale(); i++)
	{
		if (time < animationNode->scales[i].time / ticksPerSecond)
			return i;
	}

	// Return the last index if not found as it usually means we have reach the end
	return animationNode->getNumScale() - 1;
}

void AnimationComponent::updateCurrentAnimationKeyIndex(const Animation* animation)
{
	for (auto it = animation->animationNodes.begin(); it != animation->animationNodes.end(); it++)
	{
		const std::string& nodeName = it->first;
		const AnimationNode& animationNode = it->second;

		u32 currentPositionIndex = positionIndicies[selectedAnimationIndex].at(nodeName);
		u32 currentRotationIndex = rotationIndicies[selectedAnimationIndex].at(nodeName);
		u32 currentScaleIndex = scaleIndicies[selectedAnimationIndex].at(nodeName);

		positionIndicies[selectedAnimationIndex][nodeName] = getPositionIndex(animation, &animationNode, currentPositionIndex);
		rotationIndicies[selectedAnimationIndex][nodeName] = getRotationIndex(animation, &animationNode, currentRotationIndex);
		scaleIndicies[selectedAnimationIndex][nodeName] = getScaleIndex(animation, &animationNode, currentScaleIndex);
	}
}

void AnimationComponent::updateWorldTransformation()
{

}

void AnimationComponent::animationReset()
{
	accumulator = 0;
	time = 0;

	for (auto it = positionIndicies[selectedAnimationIndex].begin(); it != positionIndicies[selectedAnimationIndex].end(); it++)
	{
		std::string nodeName = it->first;

		positionIndicies[selectedAnimationIndex][nodeName] = 0;
		rotationIndicies[selectedAnimationIndex][nodeName] = 0;
		scaleIndicies[selectedAnimationIndex][nodeName] = 0;
	}
}