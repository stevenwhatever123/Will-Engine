#include "pch.h"
#include "Core/EngineGui/MaterialPanel.h"

void WillEngine::EngineGui::MaterialPanel::update(std::vector<Material*>& materials, GameState* gameState)
{
	ImGui::Begin("Material Viewer");

	std::string phongMaterials[4] = { "Emissive", "Ambient", "Diffuse", "Specular" };
	std::string brdfMaterials[5] = { "Emissive", "Ambient", "Albedo", "Metallic", "Roughness" };

	for (u32 i = 0; i < materials.size(); i++)
	{
		if (ImGui::TreeNode(materials[i]->name.c_str()))
		{
			ImGui::PushID(i);

			// BRDF materials
			if (ImGui::BeginTable("BRDF material", 1, ImGuiTableFlags_BordersV))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("BRDF material properties");

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				vec4 emissiveTemp = materials[i]->brdfMaterialUniform.emissive;
				ImGui::DragFloat3("Emissive", &materials[i]->brdfMaterialUniform.emissive.x, 0.01f, 0, 1);
				vec4 emissiveDiff = glm::epsilonNotEqual(emissiveTemp, materials[i]->brdfMaterialUniform.emissive, 0.0001f);
				if (emissiveDiff.x || emissiveDiff.y || emissiveDiff.z)
				{
					gameState->materialUpdateInfo.updateColor = true;
					gameState->materialUpdateInfo.materialIndex = i;
					gameState->materialUpdateInfo.textureIndex = 0;
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				vec4 ambientTemp = materials[i]->brdfMaterialUniform.ambient;
				ImGui::DragFloat3("Ambient", &materials[i]->brdfMaterialUniform.ambient.x, 0.01f, 0, 1);
				vec4 ambientDiff = glm::epsilonNotEqual(ambientTemp, materials[i]->brdfMaterialUniform.ambient, 0.0001f);
				if (ambientDiff.x || ambientDiff.y || ambientDiff.z)
				{
					gameState->materialUpdateInfo.updateColor = true;
					gameState->materialUpdateInfo.materialIndex = i;
					gameState->materialUpdateInfo.textureIndex = 1;
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				vec4 albedoTemp = materials[i]->brdfMaterialUniform.albedo;
				ImGui::DragFloat3("Albedo", &materials[i]->brdfMaterialUniform.albedo.x, 0.01f, 0, 1);
				vec4 albedoDiff = glm::epsilonNotEqual(albedoTemp, materials[i]->brdfMaterialUniform.albedo, 0.0001f);
				if (albedoDiff.x || albedoDiff.y || albedoDiff.z)
				{
					gameState->materialUpdateInfo.updateColor = true;
					gameState->materialUpdateInfo.materialIndex = i;
					gameState->materialUpdateInfo.textureIndex = 2;
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				f32 metallicTemp = materials[i]->brdfMaterialUniform.metallic;
				ImGui::DragFloat("Metallic", &materials[i]->brdfMaterialUniform.metallic, 0.01f, 0, 1);
				f32 metallicDiff = glm::epsilonNotEqual(metallicTemp, materials[i]->brdfMaterialUniform.metallic, 0.0001f);
				if (metallicDiff)
				{
					gameState->materialUpdateInfo.updateColor = true;
					gameState->materialUpdateInfo.materialIndex = i;
					gameState->materialUpdateInfo.textureIndex = 3;
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				f32 roughnessTemp = materials[i]->brdfMaterialUniform.roughness;
				ImGui::DragFloat("Roughness", &materials[i]->brdfMaterialUniform.roughness, 0.01f, 0, 1);
				f32 roughnessDiff = glm::epsilonNotEqual(roughnessTemp, materials[i]->brdfMaterialUniform.roughness, 0.0001f);
				if (roughnessDiff)
				{
					gameState->materialUpdateInfo.updateColor = true;
					gameState->materialUpdateInfo.materialIndex = i;
					gameState->materialUpdateInfo.textureIndex = 4;
				}

				ImGui::EndTable();
			}

			// BRDF materials
			for (u32 j = 0; j < 5; j++)
			{
				ImGui::PushID(j);

				ImGui::Text(brdfMaterials[j].c_str());

				bool lastUseTexture = materials[i]->brdfTextures[j].useTexture;
				ImGui::Checkbox("Use Texture", &materials[i]->brdfTextures[j].useTexture);

				bool checkBoxChanged = (lastUseTexture == true && materials[i]->brdfTextures[j].useTexture == false)
					|| (lastUseTexture == false && materials[i]->brdfTextures[j].useTexture == true);

				if (checkBoxChanged)
				{
					if (materials[i]->brdfTextures[j].useTexture)
					{
						gameState->materialUpdateInfo.updateTexture = true;
						gameState->materialUpdateInfo.materialIndex = i;
						gameState->materialUpdateInfo.textureIndex = j;
					}
					else
					{
						gameState->materialUpdateInfo.updateColor = true;
						gameState->materialUpdateInfo.materialIndex = i;
						gameState->materialUpdateInfo.textureIndex = j;
					}
				}

				if (materials[i]->hasTexture(j, materials[i]->brdfTextures) || materials[i]->brdfTextures[j].useTexture)
				{
					ImGui::Text("Texture path: %s", materials[i]->brdfTextures[j].texture_path.c_str());
					if (ImGui::ImageButton((ImTextureID)materials[i]->brdfTextures[j].imguiTextureDescriptorSet, ImVec2(150, 150)))
					{
						bool readSuccess;
						std::string filename;

						std::tie(readSuccess, filename) = WillEngine::Utils::selectFile();

						if (!readSuccess)
						{
							printf("Failed to read %s\n", filename.c_str());

							gameState->materialUpdateInfo.updateTexture = false;
							gameState->materialUpdateInfo.materialIndex = 0;
							gameState->materialUpdateInfo.textureIndex = 0;
							gameState->materialUpdateInfo.textureFilepath = "";
						}
						else
						{
							gameState->materialUpdateInfo.updateTexture = true;
							gameState->materialUpdateInfo.materialIndex = i;
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