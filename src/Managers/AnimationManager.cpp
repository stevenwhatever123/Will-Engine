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
		bool tickUpdated = false;

		AnimationComponent* animationComp = animationsToUpdate.front();
		u32 animationId = animationComp->getCurrentAnimationId();

		// Check if the animation is supposed to be played and whether it is a valid id
		if (animationComp->isPlayingAnimation() || animationId < 1)
		{
			animationsToUpdate.pop();
			continue;
		}

		// The animation data
		Animation* animation = animations->at(animationId);

		// Get the animation Time step
		f64 timestep = 1 / animation->getTicksPerSecond();

		animationComp->increaseAccumulator(dt);
		animationComp->increaseTime(dt);

		while (animationComp->getAccumulator() >= timestep)
		{
			// Will implement this later
			if (animationComp->getTime() >= animation->getDuration())
			{
				animationComp->animationReset();
			}

			tickUpdated = true;

			animationComp->decreaseAccumulator(timestep);
		}

		if (tickUpdated)
		{
			// Find out the value of position, rotation and scale after a tick update
			animationComp->updateCurrentAnimationKeyIndex(animation);

			transformToUpdate.push(animationComp->getParent());
		}

		animationsToUpdate.pop();
	}
}