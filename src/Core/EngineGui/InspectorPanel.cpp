#include "pch.h"
#include "Core/EngineGui/InspectorPanel.h"

#include "Core/ECS/TransformComponent.h"

void WillEngine::EngineGui::InspectorPanel::update(GameState* gameState)
{
	ImGui::Begin("Inspector");

	

	u32 selectedEntityId = gameState->uiParams.selectedEntityId;

	if (selectedEntityId)
	{
		Entity* entity = gameState->gameResources.entities[selectedEntityId];

		if (entity->HasComponent<TransformComponent>())
		{
			TransformComponent* transform = entity->GetComponent<TransformComponent>();

			if (ImGui::TreeNode("Transform"))
			{
				ImGui::DragFloat3("Position", &transform->getPosition().x, 0.1f);

				ImGui::DragFloat3("Rotation", &transform->getRotation().x, 0.1f);

				ImGui::DragFloat3("Scale", &transform->getScale().x, 0.1f, 1.0f);

				ImGui::TreePop();
			}
		}

		if (entity->HasComponent<MeshComponent>())
		{
			MeshComponent* mesh = entity->GetComponent<MeshComponent>();

			if (ImGui::TreeNode("Mesh"))
			{
				if (mesh->materialIndex < 1)
				{
					// Maybe figure it out later
					ImGui::Text("Material not selected\n");
				}
				else
				{
					ImGui::Text("Material: %s", gameState->graphicsResources.materials[mesh->materialIndex]->name.c_str());
				}
					
				ImGui::TreePop();
			}
		}

		if (entity->HasComponent<LightComponent>())
		{
			LightComponent* light = entity->GetComponent<LightComponent>();

			if (ImGui::TreeNode("Light"))
			{
				ImGui::DragFloat3("Color", &light->lightUniform.color.x, 0.001f, 0, 1);

				ImGui::DragFloat("Intensity", &light->lightUniform.intensity, 0.1f, 0, 100);

				ImGui::TreePop();
			}
		}

		// Add component
		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("Components to add");

		if (ImGui::BeginPopup("Components to add"))
		{
			for (u32 i = 1; i < ComponentType::ComponentTypeCount; i++)
			{
				ComponentType type = static_cast<ComponentType>(i);

				bool hasComp = false;

				for (auto it = entity->components.begin(); it != entity->components.end(); it++)
				{
					if (it->second != nullptr && type == it->second->getType())
					{
						hasComp = true;
					}
				}

				ImGui::BeginDisabled(hasComp);

				if (ImGui::Button(componentTypeName[type].c_str(), ImVec2(ImGui::GetWindowContentRegionWidth(), 0)))
				{
					if (type == ComponentType::MeshType)
						gameState->todoTasks.meshesToAdd.push(entity);
				}

				// Gray out: End
				ImGui::EndDisabled();
			}


			ImGui::EndPopup();
		}

	}
	else
	{
		ImGui::Text("No entity selected");
	}

	ImGui::End();
}
