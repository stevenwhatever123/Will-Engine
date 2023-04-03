#pragma once
#include "Core/Animation.h"

#include "Core/ECS/AnimationComponent.h"

using namespace WillEngine;

class AnimationManager
{
public:

	std::queue<Entity*> transformToUpdate;

private:

	std::unordered_map<u32, Animation*>* animations;

	std::queue<AnimationComponent*> animationsToUpdate;

public:

	AnimationManager();
	~AnimationManager();

	// Functions that the Animation Manager is being used
	void init(std::unordered_map<u32, Animation*>& animations) { this->animations = &animations; };
	void addToQueue(AnimationComponent* animationComp) { animationsToUpdate.push(animationComp); };
	void update(float dt);

private:
};