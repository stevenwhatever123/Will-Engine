#include "pch.h"
#include "Core/ECS/SkeletalComponent.h"

using namespace WillEngine;

SkeletalComponent::SkeletalComponent():
	Component(nullptr),
	skeletalId(0)
{

}

SkeletalComponent::SkeletalComponent(Skeleton* skeleton) :
	Component(nullptr),
	skeletalId(skeleton->id)
{

}

SkeletalComponent::~SkeletalComponent()
{

}