#include "pch.h"
#include "Core/ECS/AnimationComponent.h"

using namespace WillEngine;

AnimationComponent::AnimationComponent():
	playAnimation(false),
	selectedAnimationIndex(0),
	animationIds()
{

}

AnimationComponent::~AnimationComponent()
{

}