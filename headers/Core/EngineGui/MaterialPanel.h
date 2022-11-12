#pragma once

#include "Core/Material.h"

#include "Core/GameState.h"

namespace WillEngine::EngineGui::MaterialPanel
{
	void update(std::vector<Material*>& materials, GameState* gameState);
}