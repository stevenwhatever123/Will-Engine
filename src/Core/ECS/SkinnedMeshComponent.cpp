#include "pch.h"
#include "Core/ECS/SkinnedMeshComponent.h"

using namespace WillEngine;

SkinnedMeshComponent::SkinnedMeshComponent() :
	IRenderableComponent(),
	name(""),
	meshIndicies(),
	materialIndicies(0)
{

}

SkinnedMeshComponent::SkinnedMeshComponent(const SkinnedMeshComponent* skinnedMeshComp) :
	name("Name"),
	meshIndicies(skinnedMeshComp->meshIndicies),
	materialIndicies(skinnedMeshComp->materialIndicies)
{

}

SkinnedMeshComponent::~SkinnedMeshComponent()
{

}

void SkinnedMeshComponent::addMesh(Mesh* mesh, Material* material)
{
	meshIndicies.push_back(mesh->id);
	materialIndicies.push_back(material->id);
}