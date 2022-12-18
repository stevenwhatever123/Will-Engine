#include "pch.h"
#include "Core/EngineGui/MaterialPanel.h"

void WillEngine::EngineGui::MaterialPanel::update(GameState* gameState)
{
	ImGui::Begin("Material Viewer");

	std::string phongMaterials[4] = { "Emissive", "Ambient", "Diffuse", "Specular" };
	std::string brdfMaterials[5] = { "Emissive", "Ambient", "Albedo", "Metallic", "Roughness" };

	std::map<u32, Material*> materials = gameState->graphicsResources.materials;

	for(auto it = materials.begin(); it != materials.end(); it++)
	{
		u32 materialId = it->first;
		Material* material = it->second;

		// Check if the material really exist


		if (ImGui::TreeNode(material->name.c_str()))
		{
			ImGui::PushID(materialId);

			// BRDF materials
			if (ImGui::BeginTable("BRDF material", 1, ImGuiTableFlags_BordersV))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("BRDF material properties");

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				vec4 emissiveTemp = material->brdfMaterialUniform.emissive;
				ImGui::DragFloat3("Emissive", &material->brdfMaterialUniform.emissive.x, 0.01f, 0, 1);
				vec4 emissiveDiff = glm::epsilonNotEqual(emissiveTemp, material->brdfMaterialUniform.emissive, 0.0001f);
				if (emissiveDiff.x || emissiveDiff.y || emissiveDiff.z)
				{
					gameState->materialUpdateInfo.updateColor = true;
					gameState->materialUpdateInfo.materialId = materialId;
					gameState->materialUpdateInfo.textureIndex = 0;
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				vec4 ambientTemp = material->brdfMaterialUniform.ambient;
				ImGui::DragFloat3("Ambient", &material->brdfMaterialUniform.ambient.x, 0.01f, 0, 1);
				vec4 ambientDiff = glm::epsilonNotEqual(ambientTemp, material->brdfMaterialUniform.ambient, 0.0001f);
				if (ambientDiff.x || ambientDiff.y || ambientDiff.z)
				{
					gameState->materialUpdateInfo.updateColor = true;
					gameState->materialUpdateInfo.materialId = materialId;
					gameState->materialUpdateInfo.textureIndex = 1;
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				vec4 albedoTemp = material->brdfMaterialUniform.albedo;
				ImGui::DragFloat3("Albedo", &material->brdfMaterialUniform.albedo.x, 0.01f, 0, 1);
				vec4 albedoDiff = glm::epsilonNotEqual(albedoTemp, material->brdfMaterialUniform.albedo, 0.0001f);
				if (albedoDiff.x || albedoDiff.y || albedoDiff.z)
				{
					gameState->materialUpdateInfo.updateColor = true;
					gameState->materialUpdateInfo.materialId = materialId;
					gameState->materialUpdateInfo.textureIndex = 2;
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				f32 metallicTemp = material->brdfMaterialUniform.metallic;
				ImGui::DragFloat("Metallic", &material->brdfMaterialUniform.metallic, 0.01f, 0, 1);
				f32 metallicDiff = glm::epsilonNotEqual(metallicTemp, material->brdfMaterialUniform.metallic, 0.0001f);
				if (metallicDiff)
				{
					gameState->materialUpdateInfo.updateColor = true;
					gameState->materialUpdateInfo.materialId = materialId;
					gameState->materialUpdateInfo.textureIndex = 3;
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				f32 roughnessTemp = material->brdfMaterialUniform.roughness;
				ImGui::DragFloat("Roughness", &material->brdfMaterialUniform.roughness, 0.01f, 0, 1);
				f32 roughnessDiff = glm::epsilonNotEqual(roughnessTemp, material->brdfMaterialUniform.roughness, 0.0001f);
				if (roughnessDiff)
				{
					gameState->materialUpdateInfo.updateColor = true;
					gameState->materialUpdateInfo.materialId = materialId;
					gameState->materialUpdateInfo.textureIndex = 4;
				}

				ImGui::EndTable();
			}

			// BRDF materials
			for (u32 j = 0; j < 5; j++)
			{
				ImGui::PushID(j);

				ImGui::Text(brdfMaterials[j].c_str());

				bool lastUseTexture = material->brdfTextures[j].useTexture;
				ImGui::Checkbox("Use Texture", &material->brdfTextures[j].useTexture);

				bool checkBoxChanged = (lastUseTexture == true && material->brdfTextures[j].useTexture == false)
					|| (lastUseTexture == false && material->brdfTextures[j].useTexture == true);

				if (checkBoxChanged)
				{
					if (material->brdfTextures[j].useTexture)
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

				if (material->hasTexture(j, material->brdfTextures) || material->brdfTextures[j].useTexture)
				{
					ImGui::Text("Texture path: %s", material->brdfTextures[j].texture_path.c_str());
					if (ImGui::ImageButton((ImTextureID)material->brdfTextures[j].imguiTextureDescriptorSet, ImVec2(150, 150)))
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