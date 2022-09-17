#pragma once
#include<assimp/Importer.hpp>
#include "Core/Mesh.h"
#include "Core/Material.h"

namespace WillEngine::Utils
{
	std::vector<Mesh*> readModel(const char* filename);
	std::vector<Mesh*> extractScene(const aiScene* scene);

	void loadTexture(Material* material);
}