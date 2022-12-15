#pragma once
#include "Core/GameState.h"

#include "Core/ECS/Entity.h"

#include "Core/Material.h"

namespace WillEngine::EngineGui::EntitiesPanel
{
	void update(GameState* gameState, std::vector<Material*>& materials);
}