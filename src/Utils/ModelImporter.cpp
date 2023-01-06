#include "pch.h"
#include "Utils/ModelImporter.h"

#include "Utils/Image.h"

using namespace WillEngine;

std::tuple<std::vector<Mesh*>, std::map<u32, Material*>>
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
		return { std::vector<Mesh*>() , std::map<u32, Material*>() };
	}
}

std::tuple<std::vector<Mesh*>, std::map<u32, Material*>>
	WillEngine::Utils::extractScene(const char* filename, const aiScene* scene, std::vector<Entity*>* entities)
{
	const aiVector3D zero3D(0.0f, 0.0f, 0.0f);
	
	// materials with no unique id labeled
	std::vector<Material*> tempMaterials = extractMaterial(scene);

	std::vector<Mesh*> meshes = extractMesh(scene, tempMaterials);

	if (entities)
	{
		extractNodes(filename, scene, entities);
		//extractBones(scene);
	}

	// materials with unique id that is going to return
	std::map<u32, Material*> materials;
	for (auto material : tempMaterials)
	{
		materials[material->id] = material;
	}

	return { meshes, materials };
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

std::vector<Mesh*> WillEngine::Utils::extractMesh(const aiScene* scene, const std::vector<Material*> materials)
{
	const aiVector3D zero3D(0.0f, 0.0f, 0.0f);

	std::vector<Mesh*> meshes;
	meshes.reserve(scene->mNumMeshes);

	// Extract Mesh data
	for (u32 i = 0; i < scene->mNumMeshes; i++)
	{
		const aiMesh* currentAiMesh = scene->mMeshes[i];

		Mesh* mesh = new Mesh();

		mesh->name = currentAiMesh->mName.C_Str();

		// Reserve memory space
		mesh->positions.reserve(currentAiMesh->mNumVertices);
		mesh->normals.reserve(currentAiMesh->mNumVertices);
		mesh->uvs.reserve(currentAiMesh->mNumVertices);
		mesh->indicies.reserve(currentAiMesh->mNumVertices);

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

		//mesh->materialIndex = currentAiMesh->mMaterialIndex;
		mesh->materialIndex = materials[currentAiMesh->mMaterialIndex]->id;

		meshes.emplace_back(mesh);
	}

	return meshes;
}

void WillEngine::Utils::extractNodes(const char* filename, const aiScene* scene, std::vector<Entity*>* entities)
{
	aiNode* rootNode = scene->mRootNode;

	Entity* rootEntity = new Entity(filename);

	entities->push_back(rootEntity);

	printf("%s\n", rootNode->mName.C_Str());

	traverseNodeTree(rootNode, rootEntity, 1, entities);
}

void WillEngine::Utils::traverseNodeTree(const aiNode* node, Entity* parent, u8 level, std::vector<Entity*>* entities)
{
	for (u32 i = 0; i < node->mNumChildren; i++)
	{
		const aiNode* child = node->mChildren[i];

		Entity* childEntity = new Entity(parent, child->mName.C_Str());
		parent->addChild(childEntity);

		entities->push_back(childEntity);

		if(child->mNumMeshes)
			printf("%*s %s, Mesh Num: %u\n", level, "    ", child->mName.C_Str(), child->mNumMeshes);
		else
			printf("%*s %s, Mesh Num: %s\n", level, "    ", child->mName.C_Str(), "None");

		traverseNodeTree(child, childEntity, level + 1, entities);
	}
}

void WillEngine::Utils::extractBones(const aiScene* scene)
{
	for (u32 i = 0; i < scene->mNumMeshes; i++)
	{
		const aiMesh* mesh = scene->mMeshes[i];

		for (u32 j = 0; j < mesh->mNumBones; j++)
		{
			aiBone* bone = mesh->mBones[j];

			printf("Bone: %s, Vertex Count: %u\n", bone->mName.C_Str(), bone->mNumWeights);
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