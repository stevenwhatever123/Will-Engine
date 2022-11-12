#include "pch.h"

#include "Core/EngineGui/MeshPanel.h"

void WillEngine::EngineGui::MeshPanel::update(std::vector<Mesh*>& meshes, std::vector<Material*>& materials)
{
	ImGui::Begin("Meshes Viewer");

	for (u32 i = 0; i < meshes.size(); i++)
	{
		ImGui::PushID(i);

		if (ImGui::TreeNode(meshes[i]->name.c_str()))
		{

			ImGui::DragFloat3("Position", &meshes[i]->transformPosition.x, 0.1f);

			ImGui::Text("Material: %s", materials[meshes[i]->materialIndex]->name.c_str());

			ImGui::TreePop();
		}

		ImGui::PopID();
	}
	ImGui::End();
}
