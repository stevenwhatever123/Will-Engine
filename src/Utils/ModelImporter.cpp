#include "pch.h"
#include "Utils/ModelImporter.h"

#include "Core/ECS/TransformComponent.h"
#include "Core/MeshComponent.h"
#include "Core/ECS/SkinnedMeshComponent.h"
#include "Core/ECS/SkeletalComponent.h"

#include "Utils/Image.h"
#include "Utils/MathUtil.h"

#define MAX_BONE_INFLUENCE 4

using namespace WillEngine;

std::tuple<std::vector<Mesh*>, std::map<u32, Material*>, Skeleton*>
	WillEngine::Utils::readModel(const char* filepath, std::vector<Entity*>* entities)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(
		filepath,
		aiProcess_Triangulate |
		aiProcess_FlipUVs |
		aiProcess_GenNormals |
		aiProcess_JoinIdenticalVertices
	);

	// Get filename from path
	std::string filepath_s(filepath);
	std::string filenameWithExtention = filepath_s.substr(filepath_s.find_last_of("/\\") + 1);
	std::string::size_type const p(filenameWithExtention.find_last_of('.'));
	std::string filename = filenameWithExtention.substr(0, p);

	if (scene)
	{
		return WillEngine::Utils::extractScene(filename.c_str(), scene, entities);
	}
	else
	{
		return { std::vector<Mesh*>() , std::map<u32, Material*>(), nullptr };
	}
}

std::tuple<std::vector<Mesh*>, std::map<u32, Material*>, Skeleton*>
	WillEngine::Utils::extractScene(const char* filename, const aiScene* scene, std::vector<Entity*>* entities)
{
	// materials with no unique id labeled
	std::vector<Material*> tempMaterials = extractMaterial(scene);

	std::vector<Mesh*> meshes = extractMesh(scene);

	// Reassign material id to our own generated unique id
	for (u32 i = 0; i < meshes.size(); i++)
	{
		u32 materialIndex = meshes[i]->materialIndex;
		meshes[i]->materialIndex = tempMaterials[materialIndex]->id;
	}

	// materials with unique id that is going to return
	std::map<u32, Material*> materials;
	for (auto material : tempMaterials)
	{
		materials[material->id] = material;
	}

	Skeleton* skeleton = nullptr;

	if (entities)
	{
		if (checkHasBones(scene))
		{
			skeleton = extractBones(scene);
		}

		extractNodes(filename, scene, meshes, materials, skeleton, entities);
	}

	return { meshes, materials, skeleton };
}

std::vector<Material*> WillEngine::Utils::extractMaterial(const aiScene* scene)
{
	std::vector<Material*> materials;
	materials.reserve(scene->mNumMaterials);

	// Extract Material Data
	for (u32 i = 0; i < scene->mNumMaterials; i++)
	{
		const aiMaterial* currentAiMaterial = scene->mMaterials[i];

		Material* material = new Material();

		aiString materialName;
		aiReturn ret;

		ret = currentAiMaterial->Get(AI_MATKEY_NAME, materialName);
		if (ret != AI_SUCCESS) continue;

		material->name = materialName.C_Str();

		// BRDF Metallic
		//aiTextureType brdfMetallicTextureType[] = { aiTextureType_EMISSIVE, aiTextureType_AMBIENT, aiTextureType_BASE_COLOR, aiTextureType_METALNESS ,
		//	aiTextureType_DIFFUSE_ROUGHNESS };
		aiTextureType brdfMetallicTextureType[] = { aiTextureType_EMISSIVE, aiTextureType_AMBIENT, aiTextureType_DIFFUSE, aiTextureType_METALNESS ,
			aiTextureType_DIFFUSE_ROUGHNESS };
		u32 metallicTextureTypeSize = sizeof(brdfMetallicTextureType) / sizeof(brdfMetallicTextureType[0]);

		for (u32 j = 0; j < metallicTextureTypeSize; j++)
		{
			i32 numTextures = currentAiMaterial->GetTextureCount(brdfMetallicTextureType[j]);
			aiString texturePath;
			if (numTextures)
			{
				ret = currentAiMaterial->Get(AI_MATKEY_TEXTURE(brdfMetallicTextureType[j], 0), texturePath);

				if (ret == AI_SUCCESS)
				{
					material->brdfTextures[j].has_texture = true;
					material->brdfTextures[j].useTexture = true;
					material->brdfTextures[j].texture_path = texturePath.C_Str();
				}
			}
			else
			{
				material->brdfTextures[j].has_texture = false;
				material->brdfTextures[j].useTexture = false;
				material->brdfTextures[j].texture_path = "";
			}
		}

		// Color
		// Emissive
		{
			aiColor3D color(1, 1, 1);
			ret = currentAiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color);

			material->brdfMaterialUniform.emissive.x = color.r;
			material->brdfMaterialUniform.emissive.y = color.g;
			material->brdfMaterialUniform.emissive.z = color.b;
		}

		// Ambient
		{
			aiColor3D color(1, 1, 1);
			ret = currentAiMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color);

			material->brdfMaterialUniform.ambient.x = color.r;
			material->brdfMaterialUniform.ambient.y = color.g;
			material->brdfMaterialUniform.ambient.z = color.b;
		}

		// Diffuse
		{
			aiColor3D color(1, 1, 1);
			ret = currentAiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
			// BRDF
			material->brdfMaterialUniform.albedo.x = color.r;
			material->brdfMaterialUniform.albedo.y = color.g;
			material->brdfMaterialUniform.albedo.z = color.b;
		}

		u32 brdfMetallicTextureSize = sizeof(material->brdfTextures) / sizeof(material->brdfTextures[0]);
		for (u32 j = 0; j < brdfMetallicTextureSize; j++)
		{
			material->brdfTextures[j].width = 1;
			material->brdfTextures[j].height = 1;

			switch (j)
			{
			case 0:
				material->brdfTextures[j].textureImage->setImageColor(material->brdfMaterialUniform.emissive);
				break;
			case 1:
				material->brdfTextures[j].textureImage->setImageColor(material->brdfMaterialUniform.ambient);
				break;
			case 2:
				material->brdfTextures[j].textureImage->setImageColor(material->brdfMaterialUniform.albedo);
				break;
			case 3:
				// Metallic should be imported from a texture, here we use a default color
				material->brdfTextures[j].textureImage->setImageColor(material->brdfMaterialUniform.metallic);
				break;
			case 4:
				// Roughness should be imported from a texture, here we use a default color
				material->brdfTextures[j].textureImage->setImageColor(material->brdfMaterialUniform.roughness);
				break;
			}

			if (material->hasTexture(j, material->brdfTextures))
			{
				if (checkTexturePathExist(j, material->brdfTextures))
				{
					loadTexture(j, material, material->brdfTextures);
				}
				else
				{
					material->brdfTextures[j].has_texture = false;
					material->brdfTextures[j].useTexture = false;
					material->brdfTextures[j].texture_path = "";

					material->brdfTextures[j].width = 1;
					material->brdfTextures[j].height = 1;
				}
			}
		}
		materials.emplace_back(material);
	}

	return materials;
}

std::vector<Mesh*> WillEngine::Utils::extractMesh(const aiScene* scene)
{
	const aiVector3D zero3D(0.0f, 0.0f, 0.0f);

	std::vector<Mesh*> meshes;
	meshes.reserve(scene->mNumMeshes);

	for (u32 i = 0; i < scene->mNumMeshes; i++)
	{
		const aiMesh* currentAiMesh = scene->mMeshes[i];

		if (currentAiMesh->HasBones())
		{
			meshes.emplace_back(extractMeshWithBones(currentAiMesh));
		}
		else
		{
			meshes.emplace_back(extractMeshWithoutBones(currentAiMesh));
		}
	}

	return meshes;
}

Mesh* WillEngine::Utils::extractMeshWithoutBones(const aiMesh* currentAiMesh)
{
	Mesh* mesh = new Mesh();

	mesh->name = currentAiMesh->mName.C_Str();

	// Reserve memory space
	mesh->positions.reserve(currentAiMesh->mNumVertices);
	mesh->normals.reserve(currentAiMesh->mNumVertices);
	mesh->uvs.reserve(currentAiMesh->mNumVertices);
	mesh->indicies.reserve(currentAiMesh->mNumVertices);

	const aiVector3D zero3D(0.0f, 0.0f, 0.0f);

	bool hasTexture = currentAiMesh->HasTextureCoords(0);

	const aiVector3D* pVertex = currentAiMesh->mVertices;
	const aiVector3D* pNormal = currentAiMesh->mNormals;
	const aiVector3D* pUV = hasTexture ? currentAiMesh->mTextureCoords[0] : &zero3D;

	for (u64 j = 0; j < currentAiMesh->mNumVertices; j++)
	{
		mesh->positions.emplace_back(pVertex->x, pVertex->y, pVertex->z);
		mesh->normals.emplace_back(pNormal->x, pNormal->y, pNormal->z);
		mesh->uvs.emplace_back(pUV->x, pUV->y);

		pVertex++;
		pNormal++;

		if (hasTexture)
			pUV++;
	}

	const aiFace* face = currentAiMesh->mFaces;

	for (u64 j = 0; j < currentAiMesh->mNumFaces; j++)
	{
		for (u32 k = 0; k < face->mNumIndices; k++)
		{
			mesh->indicies.push_back(face->mIndices[k]);
		}

		face++;
	}

	mesh->indiciesSize = mesh->indicies.size();

	mesh->primitive = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	mesh->materialIndex = currentAiMesh->mMaterialIndex;

	return mesh;
}

Mesh* WillEngine::Utils::extractMeshWithBones(const aiMesh* currentAiMesh)
{
	SkinnedMesh* mesh = new SkinnedMesh();

	mesh->name = currentAiMesh->mName.C_Str();

	// Reserve memory space
	mesh->positions.reserve(currentAiMesh->mNumVertices);
	mesh->normals.reserve(currentAiMesh->mNumVertices);
	mesh->uvs.reserve(currentAiMesh->mNumVertices);
	mesh->boneWeights.resize(currentAiMesh->mNumVertices);
	mesh->indicies.reserve(currentAiMesh->mNumVertices);

	const aiVector3D zero3D(0.0f, 0.0f, 0.0f);

	bool hasTexture = currentAiMesh->HasTextureCoords(0);

	const aiVector3D* pVertex = currentAiMesh->mVertices;
	const aiVector3D* pNormal = currentAiMesh->mNormals;
	const aiVector3D* pUV = hasTexture ? currentAiMesh->mTextureCoords[0] : &zero3D;

	for (u64 j = 0; j < currentAiMesh->mNumVertices; j++)
	{
		mesh->positions.emplace_back(pVertex->x, pVertex->y, pVertex->z);
		mesh->normals.emplace_back(pNormal->x, pNormal->y, pNormal->z);
		mesh->uvs.emplace_back(pUV->x, pUV->y);

		pVertex++;
		pNormal++;

		if (hasTexture)
			pUV++;
	}

	const aiFace* face = currentAiMesh->mFaces;

	for (u64 j = 0; j < currentAiMesh->mNumFaces; j++)
	{
		for (u32 k = 0; k < face->mNumIndices; k++)
		{
			mesh->indicies.push_back(face->mIndices[k]);
		}

		face++;
	}

	mesh->indiciesSize = mesh->indicies.size();

	mesh->primitive = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	mesh->materialIndex = currentAiMesh->mMaterialIndex;

	// =====================================================

	extractVerticesBoneWeight(mesh, currentAiMesh);

	// =====================================================

	return (Mesh*) mesh;
}

void WillEngine::Utils::extractNodes(const char* filename, const aiScene* scene, std::vector<Mesh*> extractedMesh, std::map<u32, Material*> extractedMaterial,
	Skeleton* extractedSkeleton, std::vector<Entity*>* entities)
{
	aiNode* rootNode = scene->mRootNode;

	Entity* rootEntity = new Entity(filename);

	mat4 transformation = AssimpMat4ToGlmMat4(rootNode->mTransformation);
	vec3 position;
	vec3 rotation;
	vec3 scale;
	DecomposeMatrix(transformation, position, rotation, scale);

	TransformComponent* transformComp = new TransformComponent(rootEntity, position, rotation, scale);
	rootEntity->addComponent(transformComp);

	entities->push_back(rootEntity);

	//printf("%s\n", rootNode->mName.C_Str());

	traverseNodeTree(scene, rootNode, rootEntity, 1, extractedMesh, extractedMaterial, extractedSkeleton, entities);
}

void WillEngine::Utils::traverseNodeTree(const aiScene* scene, const aiNode* node, Entity* parent, u8 level, std::vector<Mesh*> extractedMesh, std::map<u32, Material*> extractedMaterial,
	Skeleton* extractedSkeleton, std::vector<Entity*>* entities)
{
	for (u32 i = 0; i < node->mNumChildren; i++)
	{
		const aiNode* child = node->mChildren[i];

		Entity* childEntity = new Entity(parent, child->mName.C_Str());
		parent->addChild(childEntity);

		mat4 transformation = AssimpMat4ToGlmMat4(child->mTransformation);
		vec3 position;
		vec3 rotation;
		vec3 scale;
		DecomposeMatrix(transformation, position, rotation, scale);

		TransformComponent* transformComp = new TransformComponent(childEntity, position, rotation, scale);
		childEntity->addComponent(transformComp);

		entities->push_back(childEntity);

		//if(child->mNumMeshes)
		//	printf("%*s %s, Mesh Num: %u\n", level, "    ", child->mName.C_Str(), child->mNumMeshes);
		//else
		//	printf("%*s %s, Mesh Num: %s\n", level, "    ", child->mName.C_Str(), "None");

		if (child->mNumMeshes > 0)
		{
			MeshComponent* meshComp = new MeshComponent();

			for (u32 j = 0; j < child->mNumMeshes; j++)
			{
				u32 meshIndex = child->mMeshes[j];
				u32 materialIndex = extractedMesh[meshIndex]->materialIndex;

				if (scene->mMeshes[meshIndex]->HasBones())
				{
					SkeletalComponent* skeletalComp = new SkeletalComponent(extractedSkeleton);
					childEntity->addComponent(skeletalComp);
				}

				meshComp->addMesh(extractedMesh[meshIndex], extractedMaterial[materialIndex]);
			}

			childEntity->addComponent(meshComp);
		}

		//if (child->mNumMeshes == 1)
		//{
		//	MeshComponent* meshComp = new MeshComponent();
		//	meshComp->addMesh(extractedMesh[child->mMeshes[0]]);

		//	childEntity->addComponent(meshComp);
		//}
		//else if (child->mNumMeshes > 1)
		//{
		//	MeshComponent* skinnedMeshComp = new SkinnedMeshComponent();
		//	for (u32 j = 0; j < child->mNumMeshes; j++)
		//	{
		//		u32 meshIndex = child->mMeshes[j];
		//		u32 materialIndex = extractedMesh[meshIndex]->materialIndex;

		//		skinnedMeshComp->addMesh(extractedMesh[meshIndex], extractedMaterial[materialIndex]);
		//	}
		//	
		//	childEntity->addComponent(skinnedMeshComp);
		//}

		traverseNodeTree(scene, child, childEntity, level + 1, extractedMesh, extractedMaterial, extractedSkeleton, entities);
	}
}

bool WillEngine::Utils::checkHasBones(const aiScene* scene)
{
	for (u32 i = 0; i < scene->mNumMeshes; i++)
	{
		if (scene->mMeshes[i]->HasBones())
			return true;
	}

	return false;
}

Skeleton* WillEngine::Utils::extractBones(const aiScene* scene)
{
	Skeleton* skeleton = new Skeleton();

	// Begin bone creation process
	BoneInfo::beginCreation();

	for (u32 i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* currentAiMesh = scene->mMeshes[i];

		if(!currentAiMesh->HasBones())
			continue;

		for (i32 boneIndex = 0; boneIndex < currentAiMesh->mNumBones; boneIndex++)
		{
			std::string boneName = currentAiMesh->mBones[boneIndex]->mName.C_Str();
			if (!skeleton->hasBone(boneName))
			{
				mat4 offsetMatrix = AssimpMat4ToGlmMat4(currentAiMesh->mBones[boneIndex]->mOffsetMatrix);

				BoneInfo boneInfo{};
				boneInfo.setName(boneName);
				boneInfo.setOffsetMatrix(offsetMatrix);

				skeleton->addBone(boneInfo);
			}
		}
	}

	// End bone creation process
	BoneInfo::endCreation();

	// Generate bone uniform
	skeleton->generateBoneUniform();

	return skeleton;
}

void WillEngine::Utils::extractVerticesBoneWeight(SkinnedMesh* mesh, const aiMesh* currentAiMesh)
{
	for (i32 boneIndex = 0; boneIndex < currentAiMesh->mNumBones; boneIndex++)
	{
		auto weights = currentAiMesh->mBones[boneIndex]->mWeights;
		u32 numWeights = currentAiMesh->mBones[boneIndex]->mNumWeights;
		for (u32 weightIndex = 0; weightIndex < numWeights; weightIndex++)
		{
			u32 vertexIndex = weights[weightIndex].mVertexId;
			f32 vertexWeight = (f32) weights[weightIndex].mWeight;

			assert(vertexIndex <= mesh->positions.size());
			setVertexBoneData(mesh, vertexIndex, boneIndex, vertexWeight);
		}
	}
}

void WillEngine::Utils::setVertexBoneData(SkinnedMesh* mesh, u32 index, u32 boneId, f32 weight)
{
	for (u32 i = 0; i < MAX_BONE_INFLUENCE; i++)
	{
		// Ignore this bone id and weight if it is 0
		if (weight < 0.0001f)
			break;

		if (mesh->boneWeights[index].boneIds[i] < 0)
		{
			mesh->boneWeights[index].boneIds[i] = boneId;
			mesh->boneWeights[index].weights[i] = weight;
			break;
		}
	}
}

void WillEngine::Utils::loadTexture(u32 index, Material* material, TextureDescriptorSet* textures)
{
	// Clear old memory
	textures[index].textureImage->freeImage(); 
	delete textures[index].textureImage;

	Image* image = new Image();
	image->readImage(textures[index].texture_path.c_str(), textures[index].width, textures[index].height, textures[index].numChannels);
	material->setTextureImage(index, image, textures);
}

bool WillEngine::Utils::checkTexturePathExist(u32 index, const TextureDescriptorSet* textures)
{
	return std::filesystem::exists(textures[index].texture_path.c_str());
}

Entity* WillEngine::Utils::getRootEntity(std::vector<Entity*>& entities)
{
	return entities[0];
}