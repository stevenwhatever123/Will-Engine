#pragma once
#include "Core/Animation.h"

class AnimationManager
{
public:

private:

	std::map<u32, Animation*>* animations;

	std::queue<u32> animationsToUpdate;

public:

	AnimationManager();
	~AnimationManager();

	void init(std::map<u32, Animation*>& animations) { this->animations = &animations; };

	void addToQueue(u32 id) { animationsToUpdate.push(id); };

	void update(float dt);
};