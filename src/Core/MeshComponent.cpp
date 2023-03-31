#include "pch.h"
#include "Core/MeshComponent.h"

using namespace WillEngine;

MeshComponent::MeshComponent() :
	Component(nullptr),
	name(""),
	meshIndicies(),
	materialIndicies()
{

}

MeshComponent::MeshComponent(Entity* entity):
	Component(entity),
	name(""),
	meshIndicies(),
	materialIndicies()
{

}

MeshComponent::MeshComponent(const MeshComponent* meshComp) :
	Component(meshComp->parent),
	name(meshComp->name),
	meshIndicies(meshComp->meshIndicies),
	materialIndicies(meshComp->materialIndicies)
{

}

MeshComponent::~MeshComponent()
{

}

void MeshComponent::addMesh(Mesh* mesh)
{
	meshIndicies.push_back(mesh->id);
	materialIndicies.push_back(mesh->materialIndex);
}

void MeshComponent::addMesh(Mesh* mesh, Material* material)
{
	meshIndicies.push_back(mesh->id);
	materialIndicies.push_back(material->id);
}