#include "pch.h"
#include "Core/Animation.h"

u32 Animation::idCounter = 0;

Animation::Animation():
	name(""),
	id(++idCounter),
	numTicks(0),
	ticksPerSecond(0),
	animationNodes()
{

}

Animation::Animation(std::string name, f64 numTicks, f64 ticksPerSecond):
	name(name),
	id(++idCounter),
	numTicks(numTicks),
	ticksPerSecond(ticksPerSecond),
	animationNodes()
{

}

Animation::~Animation()
{

}