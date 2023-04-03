#pragma once
#include "Core/ECS/Component.h"

#include "Core/Animation.h"

namespace WillEngine
{
	class AnimationComponent : public Component
	{
	public:

		static const ComponentType id = ComponentType::AnimationType;

	public:

		// Checking if the animation should be played
		bool playAnimation;

		// The current animation index in animationIds
		u32 selectedAnimationIndex;
		std::vector<u32> animationIds;

		// Time to keep track of where the animation is currently at
		f64 time;

		// Accumulator for tick update
		f64 accumulator;

	private:

		// Order: #Animation->AnimationNode(By Name)->index
		std::vector<std::unordered_map<std::string, u32>> positionIndicies;
		std::vector<std::unordered_map<std::string, u32>> rotationIndicies;
		std::vector<std::unordered_map<std::string, u32>> scaleIndicies;

	private:

		mat4 worldTransformation;

	public:

		AnimationComponent();
		AnimationComponent(Entity* entity);
		~AnimationComponent();

		virtual void update() {};

		// Setters and getters
		void addAnimation(Animation* animation);
		u32 getAnimationId(u32 index) { return animationIds[index]; }
		u32 getNumAnimations() const { return animationIds.size(); }
		const std::vector<u32>& getAllAnimationIds() { return animationIds; }

		void increaseAccumulator(f64 dt) { accumulator += dt; }
		void decreaseAccumulator(f64 dt) { accumulator -= dt; }
		f64 getAccumulator() const { return accumulator; }

		void increaseTime(f64 dt) { time += dt; }
		void decreaseTime(f64 dt) { time -= dt; }
		void setTime(f64 time) { this->time = time; }
		f64 getTime() const { return time; }

		u32 getCurrentAnimationId() const { return animationIds[selectedAnimationIndex]; }

		bool isPlayingAnimation() const { return playAnimation; }

		u32 getPositionKeyIndex(u32 animationIndex, std::string nodeName) const { return positionIndicies[animationIndex].at(nodeName); }
		u32 getRotationKeyIndex(u32 animationIndex, std::string nodeName) const { return rotationIndicies[animationIndex].at(nodeName); }
		u32 getScaleKeyIndex(u32 animationIndex, std::string nodeName) const { return scaleIndicies[animationIndex].at(nodeName); }

		u32 getPositionKeyIndex(std::string nodeName) const { return positionIndicies[selectedAnimationIndex].at(nodeName); }
		u32 getRotationKeyIndex(std::string nodeName) const { return rotationIndicies[selectedAnimationIndex].at(nodeName); }
		u32 getScaleKeyIndex(std::string nodeName) const { return scaleIndicies[selectedAnimationIndex].at(nodeName); }

		virtual ComponentType getType() { return id; };

	public:

		void updateCurrentAnimationKeyIndex(const Animation* animation);

		void updateWorldTransformation();
		mat4& const getWorldTransformation() { return worldTransformation; }

		void animationReset();

	private:
		// Function for calculating the animation transform
		u32 getPositionIndex(const Animation* animation, const AnimationNode* animationNode, u32 currentIndex) const;
		u32 getRotationIndex(const Animation* animation, const AnimationNode* animationNode, u32 currentIndex) const;
		u32 getScaleIndex(const Animation* animation, const AnimationNode* animationNode, u32 currentIndex) const;
	};
}