#include "pch.h"

#include "Core/EngineGui/ScenePanel.h"

void WillEngine::EngineGui::ScenePanel::update(ImTextureID image)
{
	ImGui::Begin("Scene");

	ImVec2 extent(ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

	ImGui::Image(image, extent);

	ImGui::End();
}