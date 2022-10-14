#include "pch.h"
#include "Utils/ModelImporter.h"

#include "Utils/Image.h"

std::tuple<std::vector<Mesh*>, std::vector<Material*>>
	WillEngine::Utils::readModel(const char* filename)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(
		filename,
		aiProcess_Triangulate |
		aiProcess_FlipUVs |
		aiProcess_GenSmoothNormals |
		aiProcess_JoinIdenticalVertices
	);

	if (scene)
	{
		return WillEngine::Utils::extractScene(scene);
	}
	else
	{
		return { std::vector<Mesh*>() , std::vector<Material*>() };
	}
}

std::tuple<std::vector<Mesh*>, std::vector<Material*>> 
	WillEngine::Utils::extractScene(const aiScene* scene)
{
	const aiVector3D zero3D(0.0f, 0.0f, 0.0f);

	std::vector<Mesh*> meshes;
	meshes.reserve(scene->mNumMeshes);

	std::vector<Material*> materials;
	materials.reserve(scene->mNumMaterials);

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

		mesh->materialIndex = currentAiMesh->mMaterialIndex;

		meshes.emplace_back(mesh);
	}

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

		// Texture path
		// Phong
		aiTextureType phongTextureType[] = { aiTextureType_EMISSIVE, aiTextureType_AMBIENT, aiTextureType_DIFFUSE, aiTextureType_SPECULAR };
		u32 phongTextureTypeSize = sizeof(phongTextureType) / sizeof(phongTextureType[0]);

		for (u32 j = 0; j < phongTextureTypeSize; j++)
		{
			i32 numTextures = currentAiMaterial->GetTextureCount(phongTextureType[j]);
			aiString texturePath;
			if (numTextures)
			{
				ret = currentAiMaterial->Get(AI_MATKEY_TEXTURE(phongTextureType[j], 0), texturePath);

				if (ret == AI_SUCCESS)
				{
					material->textures[j].has_texture = true;
					material->textures[j].useTexture = true;
					material->textures[j].texture_path = texturePath.C_Str();
				}
			}
			else
			{
				material->textures[j].has_texture = false;
				material->textures[j].useTexture = false;
				material->textures[j].texture_path = "";
			}
		}

		// BRDF Metallic
		aiTextureType brdfMetallicTextureType[] = { aiTextureType_EMISSIVE, aiTextureType_AMBIENT, aiTextureType_BASE_COLOR, aiTextureType_METALNESS ,
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
		// Phong
		{
			aiColor3D color(1, 1, 1);
			ret = currentAiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color);
			material->phongMaterialUniform.emissiveColor.x = color.r;
			material->phongMaterialUniform.emissiveColor.y = color.g;
			material->phongMaterialUniform.emissiveColor.z = color.b;
		}

		// Ambient
		{
			aiColor3D color(1, 1, 1);
			ret = currentAiMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color);
			material->phongMaterialUniform.ambientColor.x = color.r;
			material->phongMaterialUniform.ambientColor.y = color.g;
			material->phongMaterialUniform.ambientColor.z = color.b;
		}

		// Diffuse
		{
			// Phong
			aiColor3D color(1, 1, 1);
			ret = currentAiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
			material->phongMaterialUniform.diffuseColor.x = color.r;
			material->phongMaterialUniform.diffuseColor.y = color.g;
			material->phongMaterialUniform.diffuseColor.z = color.b;
			// BRDF
			material->brdfMaterialUniform.albedo.x = color.r;
			material->brdfMaterialUniform.albedo.y = color.g;
			material->brdfMaterialUniform.albedo.z = color.b;
		}

		// Specular
		{
			aiColor3D color(1, 1, 1);
			ret = currentAiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);
			material->phongMaterialUniform.specularColor.x = color.r;
			material->phongMaterialUniform.specularColor.y = color.g;
			material->phongMaterialUniform.specularColor.z = color.b;
			// BRDF
			// DO LATER
		}
		

		// Load texture
		// Phong
		u32 phongTextureSize = sizeof(material->textures) / sizeof(material->textures[0]);
		for (u32 j = 0; j < phongTextureSize; j++)
		{
			material->textures[j].width = 1;
			material->textures[j].height = 1;

			switch (j)
			{
			case 0:
				material->textures[j].textureImage->setImageColor(material->phongMaterialUniform.emissiveColor);
				break;
			case 1:
				material->textures[j].textureImage->setImageColor(material->phongMaterialUniform.ambientColor);
				break;
			case 2:
				material->textures[j].textureImage->setImageColor(material->phongMaterialUniform.diffuseColor);
				break;
			case 3:
				material->textures[j].textureImage->setImageColor(material->phongMaterialUniform.specularColor);
				break;
			}

			if (material->hasTexture(j, material->textures))
			{
				if (checkTexturePathExist(j, material->textures))
				{
					loadTexture(j, material, material->textures);
				}
				else
				{
					material->textures[j].has_texture = false;
					material->textures[j].useTexture = false;
					material->textures[j].texture_path = "";

					material->textures[j].width = 1;
					material->textures[j].height = 1;
				}
			}
		}

		u32 brdfMetallicTextureSize = sizeof(material->brdfTextures) / sizeof(material->brdfTextures[0]);
		for (u32 j = 0; j < brdfMetallicTextureSize; j++)
		{
			material->brdfTextures[j].width = 1;
			material->brdfTextures[j].height = 1;

			switch (j)
			{
			case 0:
				material->brdfTextures[j].textureImage->setImageColor(material->phongMaterialUniform.emissiveColor);
				break;
			case 1:
				material->brdfTextures[j].textureImage->setImageColor(material->phongMaterialUniform.ambientColor);
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

	return { meshes, materials };
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