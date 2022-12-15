#include "pch.h"
#include "Core/EngineGui/EntitiesPanel.h"

#include "Core/ECS/TransformComponent.h"
#include "Core/MeshComponent.h"
#include "Core/LightComponent.h"

using namespace WillEngine::EngineGui;

void EntitiesPanel::update(GameState* gameState, std::vector<Material*>& materials)
{
	std::map<u32, Entity*>& entities = gameState->gameResources.entities;

	ImGui::Begin("Entities");

	for (auto it = entities.begin(); it != entities.end(); it++)
	{
		u32 entityId = it->first;
		Entity* entity = it->second;

		ImGui::PushID(entityId);

		if (ImGui::Selectable(entity->name.c_str(), gameState->uiParams.selectedEntityId == entityId))
		{
			gameState->uiParams.selectedEntityId = entityId;
		}

		ImGui::PopID();
	}

	ImGui::End();
}
