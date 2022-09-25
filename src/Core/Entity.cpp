#include "pch.h"
#include "Core/Entity.h"

Entity::Entity() :
	mesh(nullptr),
	material(nullptr)
{

}

Entity::Entity(Mesh* mesh, Material* material):
	mesh(mesh),
	material(material)
{

}

Entity::~Entity()
{

}