#include "pch.h"
#include "Core/EngineGui/EntitiesPanel.h"

#include "Core/ECS/TransformComponent.h"

using namespace WillEngine::EngineGui;

void EntitiesPanel::update(std::vector<Entity*>& entities, std::vector<Material*>& materials)
{
	ImGui::Begin("Entities");

	for(u32 i = 0; i < entities.size(); i++)
	{
		ImGui::PushID(i);

		if (ImGui::TreeNode(entities[i]->name.c_str()))
		{
			MeshComponent* mesh = entities[i]->GetComponent<MeshComponent>();
			TransformComponent* transform = entities[i]->GetComponent<TransformComponent>();

			ImGui::DragFloat3("Position", &transform->getPosition().x, 0.1f);

			ImGui::DragFloat3("Rotation", &transform->getRotation().x, 0.1f);

			ImGui::DragFloat3("Scale", &transform->getScale().x, 0.1f, 1.0f);

			ImGui::Text("Material: %s", materials[mesh->materialIndex]->name.c_str());

			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	ImGui::End();
}
