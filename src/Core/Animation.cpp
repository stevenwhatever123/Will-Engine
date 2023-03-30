#include "pch.h"
#include "Core/Animation.h"

u32 Animation::idCounter = 0;

Animation::Animation():
	name(""),
	id(++idCounter),
	duration(0),
	ticksPerSecond(0),
	animationNodes(),
	time(0),
	accumulator(0)
{

}

Animation::Animation(std::string name, f64 duration, f64 ticksPerSecond):
	name(name),
	id(++idCounter),
	duration(duration),
	ticksPerSecond(ticksPerSecond),
	animationNodes(),
	time(0),
	accumulator(0)
{

}

Animation::~Animation()
{

}