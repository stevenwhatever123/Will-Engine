#include "pch.h"
#include "Core/MeshComponent.h"

using namespace WillEngine;

MeshComponent::MeshComponent() :
	name(""),
	meshIndex(0),
	materialIndex(0)
{

}

MeshComponent::MeshComponent(const Mesh* mesh) :
	name(mesh->name),
	meshIndex(mesh->id),
	materialIndex(mesh->materialIndex)
{

}

MeshComponent::~MeshComponent()
{

}

void MeshComponent::setMesh(Mesh* mesh)
{
	this->meshIndex = mesh->id;
	this->materialIndex = mesh->materialIndex;
}