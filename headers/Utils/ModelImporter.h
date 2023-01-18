#pragma once
#include<assimp/Importer.hpp>
#include "Core/Mesh.h"
#include "Core/SkinnedMesh.h"
#include "Core/Skeleton.h"
#include "Core/Material.h"

#include "Core/ECS/Entity.h"

using namespace WillEngine;

namespace WillEngine::Utils
{
	std::tuple<std::vector<Mesh*>, std::map<u32, Material*>, Skeleton*> readModel(const char* filepath, std::vector<Entity*>* entities = nullptr);
	std::tuple<std::vector<Mesh*>, std::map<u32, Material*>, Skeleton*> extractScene(const char* filename, const aiScene* scene, std::vector<Entity*>* entities = nullptr);

	std::vector<Material*> extractMaterial(const aiScene* scene);
	std::vector<Mesh*> extractMesh(const aiScene* scene);
	Mesh* extractMeshWithoutBones(const aiMesh* currentAiMesh);
	Mesh* extractMeshWithBones(const aiMesh* mesh);
	// For Skeletal Animation
	void extractNodes(const char* filename, const aiScene* scene, std::vector<Mesh*> extractedMesh, std::map<u32, Material*> extractedMaterial, std::vector<Entity*>* entities);
	void traverseNodeTree(const aiNode* node, Entity* parent, u8 level, std::vector<Mesh*> extractedMesh, std::map<u32, Material*> extractedMaterial, std::vector<Entity*>* entities);

	bool checkHasBones(const aiScene* scene);
	Skeleton* extractBones(const aiScene* scene);
	void extractVerticesBoneWeight(SkinnedMesh* mesh, const aiMesh* currentAiMesh);
	void setVertexBoneData(SkinnedMesh* mesh, u32 index, u32 boneId, float weight);

	void loadTexture(u32 index, Material* material, TextureDescriptorSet* textures);
	bool checkTexturePathExist(u32 index, const TextureDescriptorSet* textures);
	Entity* getRootEntity(std::vector<Entity*>& entities);
}