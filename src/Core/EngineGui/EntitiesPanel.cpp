#include "pch.h"
#include "Core/EngineGui/EntitiesPanel.h"

#include "Core/ECS/TransformComponent.h"
#include "Core/MeshComponent.h"
#include "Core/LightComponent.h"

using namespace WillEngine::EngineGui;

void EntitiesPanel::update(GameState* gameState)
{
	std::map<u32, Entity*>& entities = gameState->gameResources.entities;
	std::map<u32, Entity*>& rootEntities = gameState->gameResources.rootEntities;

	ImGui::Begin("Entities");

	//for (auto it = entities.begin(); it != entities.end(); it++)
	//{
	//	u32 entityId = it->first;
	//	Entity* entity = it->second;

	//	ImGui::PushID(entityId);

	//	if (ImGui::Selectable(entity->name.c_str(), gameState->uiParams.selectedEntityId == entityId))
	//	{
	//		gameState->uiParams.selectedEntityId = entityId;
	//	}

	//	ImGui::PopID();
	//}

	for (auto it = rootEntities.begin(); it != rootEntities.end(); it++)
	{
		u32 entityId = it->first;
		Entity* entity = it->second;

		ImGui::PushID(entityId);

		// Use text if there are no children
		if (!entity->hasChildren())
		{
			if (ImGui::Selectable(entity->name.c_str(), gameState->uiParams.selectedEntityId == entityId))
			{
				gameState->uiParams.selectedEntityId = entityId;
			}

			ImGui::PopID();

			continue;
		}

		// Otherwise use a tree
		if (ImGui::TreeNode(entity->name.c_str()))
		{
			for (auto child : entity->children)
			{
				traverseEntityHierarchy(child, gameState);
			}
			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	ImGui::End();
}


void EntitiesPanel::traverseEntityHierarchy(Entity* entity, GameState* gameState)
{
	u32 entityId = entity->id;

	// Use Text if there are no children
	if (!entity->hasChildren())
	{
		if (ImGui::Selectable(entity->name.c_str(), gameState->uiParams.selectedEntityId == entityId))
		{
			gameState->uiParams.selectedEntityId = entityId;
		}

		for (auto child : entity->children)
		{
			traverseEntityHierarchy(child, gameState);
		}

		return;
	}

	// Otherwise use a tree
	if (ImGui::TreeNode(entity->name.c_str()))
	{
		for (auto child : entity->children)
		{
			traverseEntityHierarchy(child, gameState);
		}

		ImGui::TreePop();
	}
}
