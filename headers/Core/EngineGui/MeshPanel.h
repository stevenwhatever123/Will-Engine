#pragma once

#include "Core/Mesh.h"
#include "Core/Material.h"

namespace WillEngine::EngineGui::MeshPanel
{
	void update(std::vector<Mesh*>& meshes, std::vector<Material*>& materials);
}