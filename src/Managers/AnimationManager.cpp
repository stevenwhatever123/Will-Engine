#include "pch.h"
#include "Managers/AnimationManager.h"

AnimationManager::AnimationManager()
{

}

AnimationManager::~AnimationManager()
{

}

void AnimationManager::update(float dt)
{
	while (!animationsToUpdate.empty())
	{
		u32 animationId = animationsToUpdate.front();
		Animation* animation = animations->find(animationId)->second;

		animationsToUpdate.pop();
	}
}