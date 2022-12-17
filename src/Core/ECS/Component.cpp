#include "pch.h"
#include "Core/ECS/Component.h"

using namespace WillEngine;

Component::Component():
	parent(nullptr)
{

}

Component::Component(Entity* entity) :
	parent(entity)
{

}

Component::~Component()
{

}