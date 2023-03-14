#include "pch.h"
#include "Core/Light.h"

#include "Core/EngineGui/InspectorPanel.h"

#include "Core/ECS/TransformComponent.h"
#include "Core/MeshComponent.h"
#include "Core/ECS/SkinnedMeshComponent.h"
#include "Core/ECS/SkeletalComponent.h"

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

				if (ImGui::DragFloat3("Position", &transform->getPosition().x, 0.1f, 0, 0, "%.7f"))
				{
					gameState->queryTasks.updateTransformation = true;
				}

				if (ImGui::DragFloat3("Rotation", &transform->getRotation().x, 0.1f, 0, 0, "%.7f"))
				{
					gameState->queryTasks.updateTransformation = true;
				}

				if (ImGui::DragFloat3("Scale", &transform->getScale().x, 0.1f, 0, 0, "%.7f"))
				{
					gameState->queryTasks.updateTransformation = true;
				}

				ImGui::TreePop();
			}
		}

		if (entity->HasComponent<MeshComponent>())
		{
			if (ImGui::TreeNode("Renderable Mesh"))
			{
				MeshComponent* meshComp = entity->GetComponent<MeshComponent>();

				for (u32 i = 0; i < meshComp->getNumMesh(); i++)
				{
					u32 meshId = meshComp->meshIndicies[i];
					u32 materialId = meshComp->materialIndicies[i];
					ImGui::Text("Mesh: %s", gameState->graphicsResources.meshes[meshId]->name.c_str());

					if(materialId)
						ImGui::Text("Material: %s", gameState->graphicsResources.materials[materialId]->name.c_str());
					else
						ImGui::Text("Material not selected\n");

					ImGui::NewLine();
				}

				ImGui::TreePop();
			}
		}

		if (entity->HasComponent<SkeletalComponent>())
		{
			SkeletalComponent* skeletalComp = entity->GetComponent<SkeletalComponent>();

			if (ImGui::TreeNode("Skeletal Component"))
			{
				ImGui::Text("Hello I have Skeleton");

				ImGui::TreePop();
			}
		}

		if (entity->HasComponent<LightComponent>())
		{
			LightComponent* lightComp = entity->GetComponent<LightComponent>();
			Light* light = gameState->graphicsResources.lights[lightComp->lightIndex];

			if (ImGui::TreeNode("Light"))
			{
				ImGui::DragFloat3("Color", &light->lightUniform.color.x, 0.001f, 0, 1);

				ImGui::DragFloat("Range", &light->lightUniform.range, 1.0f, 0, 1000000);

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
						gameState->queryTasks.meshesToAdd.push(entity);
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
