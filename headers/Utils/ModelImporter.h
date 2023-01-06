#pragma once
#include<assimp/Importer.hpp>
#include "Core/Mesh.h"
#include "Core/Material.h"

#include "Core/ECS/Entity.h"

using namespace WillEngine;

namespace WillEngine::Utils
{
	std::tuple<std::vector<Mesh*>, std::map<u32, Material*>> readModel(const char* filepath, std::vector<Entity*>* entities = nullptr);
	std::tuple<std::vector<Mesh*>, std::map<u32, Material*>> extractScene(const char* filename, const aiScene* scene, std::vector<Entity*>* entities = nullptr);

	std::vector<Material*> extractMaterial(const aiScene* scene);
	std::vector<Mesh*> extractMesh(const aiScene* scene, const std::vector<Material*> materials);
	// For Skeletal Animation
	void extractNodes(const char* filename, const aiScene* scene, std::vector<Entity*>* entities);
	void traverseNodeTree(const aiNode* node, Entity* parent, u8 level, std::vector<Entity*>* entities);

	void extractBones(const aiScene* scene);

	void loadTexture(u32 index, Material* material, TextureDescriptorSet* textures);
	bool checkTexturePathExist(u32 index, const TextureDescriptorSet* textures);
}