#include "pch.h"
#include "Core/EngineGui/EntitiesPanel.h"

#include "Core/ECS/TransformComponent.h"
#include "Core/MeshComponent.h"
#include "Core/LightComponent.h"

using namespace WillEngine::EngineGui;

void EntitiesPanel::update(GameState* gameState)
{
	std::unordered_map<u32, Entity*>& entities = gameState->gameResources.entities;
	std::unordered_map<u32, Entity*>& rootEntities = gameState->gameResources.rootEntities;

	ImGui::Begin("Entities");

	for (auto it = rootEntities.begin(); it != rootEntities.end(); it++)
	{
		u32 entityId = it->first;
		Entity* entity = it->second;

		ImGui::PushID(entityId);

		traverseEntityHierarchy(entity, gameState);

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
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow;
	bool expand = ImGui::TreeNodeEx((void*)(uintptr_t)entityId, node_flags, entity->name.c_str());

	if (ImGui::IsItemClicked())
		gameState->uiParams.selectedEntityId = entityId;

	if (expand)
	{
		for (auto child : entity->children)
		{
			traverseEntityHierarchy(child, gameState);
		}

		ImGui::TreePop();
	}
}
