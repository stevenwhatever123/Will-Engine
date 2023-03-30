#pragma once
#include "Core/ECS/Component.h"

namespace WillEngine
{
	class AnimationComponent : public Component
	{
	public:

		static const ComponentType id = ComponentType::AnimationType;

	public:

		bool playAnimation;

		u32 selectedAnimationIndex;
		std::vector<u32> animationIds;

	public:

		AnimationComponent();
		~AnimationComponent();

		virtual void update() {};

		void addAnimation(u32 id) { animationIds.push_back(id); }
		u32 getAnimationId(u32 index) { return animationIds[index]; }
		u32 getNumAnimations() const { return animationIds.size(); }

		const std::vector<u32>& getAllAnimationIds() { return animationIds; }

		virtual ComponentType getType() { return id; };
	};
}