#pragma once
#include<assimp/Importer.hpp>
#include "Core/MeshComponent.h"
#include "Core/Material.h"

using namespace WillEngine;

namespace WillEngine::Utils
{
	std::tuple<std::vector<MeshComponent*>, std::vector<Material*>> readModel(const char* filename);
	std::tuple<std::vector<MeshComponent*>, std::vector<Material*>> extractScene(const aiScene* scene);

	void loadTexture(u32 index, Material* material, TextureDescriptorSet* textures);
	bool checkTexturePathExist(u32 index, const TextureDescriptorSet* textures);
}