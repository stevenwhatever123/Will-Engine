#include "pch.h"
#include "Core/LightComponent.h"

using namespace WillEngine;

LightComponent::LightComponent() :
    Component(nullptr),
    lightIndex(0)
{

}

LightComponent::LightComponent(Entity* entity) :
    Component(entity),
    lightIndex(0)
{

}

LightComponent::LightComponent(const Light* light) :
    lightIndex(light->id)
{

}

LightComponent::~LightComponent()
{

}