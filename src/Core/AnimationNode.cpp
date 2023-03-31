#include "pch.h"
#include "Core/AnimationNode.h"

AnimationNode::AnimationNode():
	name(""),
	positions(),
	rotations(),
	scales()
{

}

AnimationNode::AnimationNode(std::string name):
	name(name),
	positions(),
	rotations(),
	scales()
{

}

AnimationNode::AnimationNode(std::string name, std::vector<KeyData> position, std::vector<QuatData> rotation, std::vector<KeyData> scale):
	name(name),
	positions(position),
	rotations(rotation),
	scales(scale)
{

}

AnimationNode::~AnimationNode()
{

}