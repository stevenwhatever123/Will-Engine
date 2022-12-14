#include "pch.h"
#include "Core/EngineGui/EntitiesPanel.h"

#include "Core/ECS/TransformComponent.h"
#include "Core/MeshComponent.h"
#include "Core/LightComponent.h"

using namespace WillEngine::EngineGui;

void EntitiesPanel::update(std::vector<Entity*>& entities, std::vector<Material*>& materials)
{
	ImGui::Begin("Entities");

	for(u32 i = 0; i < entities.size(); i++)
	{
		ImGui::PushID(i);

		if (ImGui::TreeNode(entities[i]->name.c_str()))
		{
			if (entities[i]->HasComponent<TransformComponent>())
			{
				TransformComponent* transform = entities[i]->GetComponent<TransformComponent>();

				ImGui::DragFloat3("Position", &transform->getPosition().x, 0.1f);

				ImGui::DragFloat3("Rotation", &transform->getRotation().x, 0.1f);

				ImGui::DragFloat3("Scale", &transform->getScale().x, 0.1f, 1.0f);
			}

			if (entities[i]->HasComponent<MeshComponent>())
			{
				MeshComponent* mesh = entities[i]->GetComponent<MeshComponent>();

				ImGui::Text("Material: %s", materials[mesh->materialIndex]->name.c_str());
			}

			if (entities[i]->HasComponent<LightComponent>())
			{
				LightComponent* light = entities[i]->GetComponent<LightComponent>();

				ImGui::DragFloat("Intensity", &light->lightUniform.intensity, 0.1f, 0, 100);
			}

			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	ImGui::End();
}
