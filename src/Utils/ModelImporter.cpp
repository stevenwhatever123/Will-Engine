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
		// Emissive
		i32 numEmissiveTextures = currentAiMaterial->GetTextureCount(aiTextureType_EMISSIVE);
		aiString emissiveTexturePath;
		if (numEmissiveTextures)
		{
			ret = currentAiMaterial->Get(AI_MATKEY_TEXTURE(aiTextureType_EMISSIVE, 0), emissiveTexturePath);

			if (ret == AI_SUCCESS)
			{
				material->textures[0].has_texture = true;
				material->textures[0].useTexture = true;
				material->textures[0].texture_path = emissiveTexturePath.C_Str();
			}
		}
		else
		{
			material->textures[0].has_texture = false;
			material->textures[0].useTexture = false;
			material->textures[0].texture_path = "";
		}
		
		// Ambient
		i32 numAmbientTextures = currentAiMaterial->GetTextureCount(aiTextureType_AMBIENT);
		aiString ambientTexturePath;
		if (numAmbientTextures)
		{
			ret = currentAiMaterial->Get(AI_MATKEY_TEXTURE(aiTextureType_AMBIENT, 0), ambientTexturePath);

			if (ret == AI_SUCCESS)
			{
				material->textures[1].has_texture = true;
				material->textures[1].useTexture = true;
				material->textures[1].texture_path = ambientTexturePath.C_Str();
			}
		}
		else
		{
			material->textures[1].has_texture = false;
			material->textures[1].useTexture = false;
			material->textures[1].texture_path = "";
		}

		// Diffuse
		i32 numDiffuseTextures = currentAiMaterial->GetTextureCount(aiTextureType_DIFFUSE);
		aiString diffuseTexturePath;
		if (numDiffuseTextures)
		{
			ret = currentAiMaterial->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), diffuseTexturePath);

			if (ret == AI_SUCCESS)
			{
				material->textures[2].has_texture = true;
				material->textures[2].useTexture = true;
				material->textures[2].texture_path = diffuseTexturePath.C_Str();
			}
		}
		else
		{
			material->textures[2].has_texture = false;
			material->textures[2].useTexture = false;
			material->textures[2].texture_path = "";
		}

		// Specular
		i32 numSpecularTextures = currentAiMaterial->GetTextureCount(aiTextureType_SPECULAR);
		aiString specularTexturePath;
		if (numSpecularTextures)
		{
			ret = currentAiMaterial->Get(AI_MATKEY_TEXTURE(aiTextureType_SPECULAR, 0), specularTexturePath);

			if (ret == AI_SUCCESS)
			{
				material->textures[3].has_texture = true;
				material->textures[3].useTexture = true;
				material->textures[3].texture_path = specularTexturePath.C_Str();
			}
		}
		else
		{
			material->textures[3].has_texture = false;
			material->textures[3].useTexture = false;
			material->textures[3].texture_path = "";
		}


		// Color
		// Emissive
		aiColor3D emissiveColor(1, 1, 1);
		ret = currentAiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor);
		material->phongMaterialUniform.emissiveColor.x = emissiveColor.r;
		material->phongMaterialUniform.emissiveColor.y = emissiveColor.g;
		material->phongMaterialUniform.emissiveColor.z = emissiveColor.b;

		// Ambient
		aiColor3D ambientColor(1, 1, 1);
		ret = currentAiMaterial->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor);
		material->phongMaterialUniform.ambientColor.x = ambientColor.r;
		material->phongMaterialUniform.ambientColor.y = ambientColor.g;
		material->phongMaterialUniform.ambientColor.z = ambientColor.b;

		// Diffuse
		aiColor3D diffuseColor(1, 1, 1);
		ret = currentAiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
		material->phongMaterialUniform.diffuseColor.x = diffuseColor.r;
		material->phongMaterialUniform.diffuseColor.y = diffuseColor.g;
		material->phongMaterialUniform.diffuseColor.z = diffuseColor.b;

		// Specular
		aiColor3D specularColor(1, 1, 1);
		ret = currentAiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
		material->phongMaterialUniform.specularColor.x = specularColor.r;
		material->phongMaterialUniform.specularColor.y = specularColor.g;
		material->phongMaterialUniform.specularColor.z = specularColor.b;

		for (u32 i = 0; i < 4; i++)
		{
			material->textures[i].width = 1;
			material->textures[i].height = 1;
			Image* image = new Image();
			material->setTextureImage(i, image);

			switch (i)
			{
			case 0:
				material->textures[i].textureImage->setImageColor(material->phongMaterialUniform.emissiveColor);
				break;
			case 1:
				material->textures[i].textureImage->setImageColor(material->phongMaterialUniform.ambientColor);
				break;
			case 2:
				material->textures[i].textureImage->setImageColor(material->phongMaterialUniform.diffuseColor);
				break;
			case 3:
				material->textures[i].textureImage->setImageColor(material->phongMaterialUniform.specularColor);
				break;
			}

			if (material->hasTexture(i))
			{
				loadTexture(i, material);

				// Use the color as the texture if we cannot load the texture
				if (!material->textures[i].textureImage->data)
				{
					material->textures[i].has_texture = false;
					material->textures[i].useTexture = false;
					material->textures[i].texture_path = "";

					material->textures[i].width = 1;
					material->textures[i].height = 1;

					switch (i)
					{
					case 0:
						material->textures[i].textureImage->setImageColor(material->phongMaterialUniform.emissiveColor);
						break;
					case 1:
						material->textures[i].textureImage->setImageColor(material->phongMaterialUniform.ambientColor);
						break;
					case 2:
						material->textures[i].textureImage->setImageColor(material->phongMaterialUniform.diffuseColor);
						break;
					case 3:
						material->textures[i].textureImage->setImageColor(material->phongMaterialUniform.specularColor);
						break;
					}
				}
			}
		}

		materials.emplace_back(material);
	}

	return { meshes, materials };
}

void WillEngine::Utils::loadTexture(u32 index, Material* material)
{
	Image* image = new Image();
	image->readImage(material->getTexturePath(index), material->textures[index].width, material->textures[index].height,
		material->textures[index].numChannels);

	material->setTextureImage(index, image);
}