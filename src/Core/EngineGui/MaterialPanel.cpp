#include "pch.h"
#include "Core/EngineGui/MaterialPanel.h"

void WillEngine::EngineGui::MaterialPanel::update(GameState* gameState)
{
	ImGui::Begin("Material Viewer");

	std::map<u32, Material*> materials = gameState->graphicsResources.materials;

	for(auto it = materials.begin(); it != materials.end(); it++)
	{
		u32 materialId = it->first;
		Material* material = it->second;

		// Check if the material really exist
		if (materialId < 1)
			continue;

		if (ImGui::TreeNode(material->name.c_str()))
		{
			ImGui::PushID(materialId);

			// BRDF Materials
			if (ImGui::BeginTable("Material", 1, ImGuiTableFlags_BordersV))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Material properties");

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				vec4 emissiveTemp = material->materialUniform.emissive;
				ImGui::DragFloat3("Emissive", &material->materialUniform.emissive.x, 0.01f, 0, 1);
				vec4 emissiveDiff = glm::epsilonNotEqual(emissiveTemp, material->materialUniform.emissive, 0.0001f);
				if (emissiveDiff.x || emissiveDiff.y || emissiveDiff.z)
				{
					gameState->materialUpdateInfo.updateColor = true;
					gameState->materialUpdateInfo.materialId = materialId;
					gameState->materialUpdateInfo.textureIndex = 0;
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				vec4 albedoTemp = material->materialUniform.albedo;
				ImGui::DragFloat3("Albedo", &material->materialUniform.albedo.x, 0.01f, 0, 1);
				vec4 albedoDiff = glm::epsilonNotEqual(albedoTemp, material->materialUniform.albedo, 0.0001f);
				if (albedoDiff.x || albedoDiff.y || albedoDiff.z)
				{
					gameState->materialUpdateInfo.updateColor = true;
					gameState->materialUpdateInfo.materialId = materialId;
					gameState->materialUpdateInfo.textureIndex = 1;
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				f32 metallicTemp = material->materialUniform.metallic;
				ImGui::DragFloat("Metallic", &material->materialUniform.metallic, 0.01f, 0, 1);
				f32 metallicDiff = glm::epsilonNotEqual(metallicTemp, material->materialUniform.metallic, 0.0001f);
				if (metallicDiff)
				{
					gameState->materialUpdateInfo.updateColor = true;
					gameState->materialUpdateInfo.materialId = materialId;
					gameState->materialUpdateInfo.textureIndex = 2;
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				f32 roughnessTemp = material->materialUniform.roughness;
				ImGui::DragFloat("Roughness", &material->materialUniform.roughness, 0.01f, 0, 1);
				f32 roughnessDiff = glm::epsilonNotEqual(roughnessTemp, material->materialUniform.roughness, 0.0001f);
				if (roughnessDiff)
				{
					gameState->materialUpdateInfo.updateColor = true;
					gameState->materialUpdateInfo.materialId = materialId;
					gameState->materialUpdateInfo.textureIndex = 3;
				}

				ImGui::EndTable();
			}

			// BRDF materials
			for (u32 j = 0; j < Material::TEXTURE_SIZE; j++)
			{
				ImGui::PushID(j);

				ImGui::Text(Material::TEXTURE_TYPE_NAME[j].c_str());

				bool lastUseTexture = material->textures[j].useTexture;
				ImGui::Checkbox("Use Texture", &material->textures[j].useTexture);

				bool checkBoxChanged = (lastUseTexture == true && material->textures[j].useTexture == false)
					|| (lastUseTexture == false && material->textures[j].useTexture == true);

				if (checkBoxChanged)
				{
					if (material->textures[j].useTexture)
					{
						gameState->materialUpdateInfo.updateTexture = true;
						gameState->materialUpdateInfo.materialId = materialId;
						gameState->materialUpdateInfo.textureIndex = j;
					}
					else
					{
						gameState->materialUpdateInfo.updateColor = true;
						gameState->materialUpdateInfo.materialId = materialId;
						gameState->materialUpdateInfo.textureIndex = j;
					}
				}

				if (material->hasTexture(j, material->textures) || material->textures[j].useTexture)
				{
					ImGui::Text("Texture path: %s", material->textures[j].texture_path.c_str());
					if (ImGui::ImageButton((ImTextureID)material->textures[j].imguiTextureDescriptorSet, ImVec2(150, 150)))
					{
						bool readSuccess;
						std::string filename;

						std::tie(readSuccess, filename) = WillEngine::Utils::selectFile();

						if (!readSuccess)
						{
							printf("Failed to read %s\n", filename.c_str());

							gameState->materialUpdateInfo.updateTexture = false;
							gameState->materialUpdateInfo.materialId = 0;
							gameState->materialUpdateInfo.textureIndex = 0;
							gameState->materialUpdateInfo.textureFilepath = "";
						}
						else
						{
							gameState->materialUpdateInfo.updateTexture = true;
							gameState->materialUpdateInfo.materialId = materialId;
							gameState->materialUpdateInfo.textureIndex = j;
							gameState->materialUpdateInfo.textureFilepath = filename;
						}
					}
				}

				ImGui::PopID();
			}

			ImGui::PopID();

			ImGui::TreePop();
		}
	}

	ImGui::End();
}