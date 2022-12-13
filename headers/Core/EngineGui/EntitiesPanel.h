#pragma once

#include "Core/ECS/Entity.h"

#include "Core/MeshComponent.h"
#include "Core/Material.h"

namespace WillEngine::EngineGui::EntitiesPanel
{
	void update(std::vector<Entity*>& entities, std::vector<Material*>& materials);
}