#pragma once
#include<assimp/Importer.hpp>
#include "Core/Mesh.h"
#include "Core/Material.h"

using namespace WillEngine;

namespace WillEngine::Utils
{
	std::tuple<std::vector<Mesh*>, std::map<u32, Material*>> readModel(const char* filename);
	std::tuple<std::vector<Mesh*>, std::map<u32, Material*>> extractScene(const aiScene* scene);

	std::vector<Material*> extractMaterial(const aiScene* scene);
	std::vector<Mesh*> extractMesh(const aiScene* scene, const std::vector<Material*> materials);

	void loadTexture(u32 index, Material* material, TextureDescriptorSet* textures);
	bool checkTexturePathExist(u32 index, const TextureDescriptorSet* textures);
}