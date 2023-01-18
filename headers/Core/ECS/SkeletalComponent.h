#pragma once
#include "Core/ECS/Component.h"
#include "Core/Skeleton.h"

namespace WillEngine
{
	class SkeletalComponent : public Component
	{
	public:

		static const ComponentType id = ComponentType::SkeletalType;

	public:

		u32 skeletalId;

	public:

		SkeletalComponent();
		SkeletalComponent(Skeleton* skeleton);
		virtual ~SkeletalComponent();

		void addSkeletal(Skeleton* skeleton) { skeletalId = skeleton->id; };

		virtual void update() {};

		virtual ComponentType getType() { return id; };
	};
}