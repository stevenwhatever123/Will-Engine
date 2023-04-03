#pragma once
#include "Core/AnimationNode.h"

class Animation
{
public:

	// Name of the animation
	std::string name;

	// Unique id for animation
	const u32 id;

	// The data imported is a double so we store it as a double
	f64 numTicks;

	// Number of ticks per second
	f64 ticksPerSecond;

	// The actual animation data
	//std::vector<AnimationNode> animationNodes;
	std::unordered_map<std::string, AnimationNode> animationNodes;

public:

private:

	// Used for generating an id for an Animation
	static u32 idCounter;

public:

	Animation();
	Animation(std::string name, f64 numTicks, f64 ticksPerSecond);
	~Animation();

	//void setNumChannels(u32 size) { animationNodes.resize(size); };

	std::string getName() const { return name; }
	f64 getNumTicks() const { return numTicks; }
	f64 getTicksPerSecond() const { return ticksPerSecond; }
	f64 getDuration() const { return numTicks / ticksPerSecond; }

	//AnimationNode& getModifiableAnimationNode(u32 index) { return animationNodes[index]; };
	//const AnimationNode& getAnimationNode(u32 index) const { return animationNodes[index]; };
	u32 getNumAnimationNode() const { return animationNodes.size(); }
};