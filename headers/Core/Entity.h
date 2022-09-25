#pragma once
#include "Core/Mesh.h"
#include "Core/Material.h"

class Entity
{
private:

	Mesh* mesh;
	Material* material;

public:

public:

	Entity();
	Entity(Mesh* mesh, Material* material);
	~Entity();

};