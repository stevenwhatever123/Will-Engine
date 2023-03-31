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

	std::vector<u32> animationNodePositionIndex;
	std::vector<u32> animationNodeRotationIndex;
	std::vector<u32> animationNodeScaleIndex;

	for (auto animationNode : animation->animationNodes)
	{
		animationNodePositionIndex.push_back(0);
		animationNodeRotationIndex.push_back(0);
		animationNodeScaleIndex.push_back(0);
	}

	positionIndicies.push_back(animationNodePositionIndex);
	rotationIndicies.push_back(animationNodeRotationIndex);
	scaleIndicies.push_back(animationNodeScaleIndex);
}

u32 AnimationComponent::getPositionIndex(const Animation* animation, const AnimationNode* animationNode, u32 currentIndex) const
{
	for (u32 i = currentIndex; i < animationNode->getNumPosition(); i++)
	{
		if (time < animationNode->positions[i].time / animation->getTicksPerSecond())
			return i;
	}

	// Return the last index if not found as it usually means we have reach the end
	return animationNode->getNumPosition() - 1;
}

u32 AnimationComponent::getRotationIndex(const Animation* animation, const AnimationNode* animationNode, u32 currentIndex) const
{
	for (u32 i = currentIndex; i < animationNode->getNumRotation(); i++)
	{
		if (time < animationNode->rotations[i].time / animation->getTicksPerSecond())
			return i;
	}

	// Return the last index if not found as it usually means we have reach the end
	return animationNode->getNumRotation() - 1;
}

u32 AnimationComponent::getScaleIndex(const Animation* animation, const AnimationNode* animationNode, u32 currentIndex) const
{
	for (u32 i = currentIndex; i < animationNode->getNumScale(); i++)
	{
		if (time < animationNode->scales[i].time / animation->getTicksPerSecond())
			return i;
	}

	// Return the last index if not found as it usually means we have reach the end
	return animationNode->getNumScale() - 1;
}

void AnimationComponent::updateCurrentAnimationKeyIndex(const Animation* animation)
{
	for (u32 i = 0; i < animation->getNumAnimationNode(); i++)
	{
		positionIndicies[selectedAnimationIndex][i] = getPositionIndex(animation, &animation->animationNodes[i], positionIndicies[selectedAnimationIndex][i]);
		rotationIndicies[selectedAnimationIndex][i] = getRotationIndex(animation, &animation->animationNodes[i], rotationIndicies[selectedAnimationIndex][i]);
		scaleIndicies[selectedAnimationIndex][i] = getScaleIndex(animation, &animation->animationNodes[i], scaleIndicies[selectedAnimationIndex][i]);
	}
}

void AnimationComponent::updateWorldTransformation()
{

}

void AnimationComponent::animationReset()
{
	accumulator = 0;
	time = 0;

	for (u32 i = 0; i < positionIndicies[selectedAnimationIndex].size(); i++)
	{
		positionIndicies[selectedAnimationIndex][i] = 0;
		rotationIndicies[selectedAnimationIndex][i] = 0;
		scaleIndicies[selectedAnimationIndex][i] = 0;
	}
}