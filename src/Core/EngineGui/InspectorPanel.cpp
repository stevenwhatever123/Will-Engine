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
				ImGui::Text("Material: %s", gameState->graphicsResources.materials[mesh->materialIndex]->name.c_str());

				ImGui::TreePop();
			}
		}

		if (entity->HasComponent<LightComponent>())
		{
			LightComponent* light = entity->GetComponent<LightComponent>();

			if (ImGui::TreeNode("Light"))
			{
				ImGui::DragFloat("Intensity", &light->lightUniform.intensity, 0.1f, 0, 100);

				ImGui::TreePop();
			}
		}
	}
	else
	{
		ImGui::Text("No entity selected");
	}

	ImGui::End();
}
