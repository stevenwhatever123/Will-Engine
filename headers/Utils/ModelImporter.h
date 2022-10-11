#pragma once
#include<assimp/Importer.hpp>
#include "Core/Mesh.h"
#include "Core/Material.h"
#include "Core/Entity.h"

namespace WillEngine::Utils
{
	std::tuple<std::vector<Mesh*>, std::vector<Material*>> readModel(const char* filename);
	std::tuple<std::vector<Mesh*>, std::vector<Material*>> extractScene(const aiScene* scene);

	void loadTexture(u32 index, Material* material);
}