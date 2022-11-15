#include "pch.h"

#include "Core/EngineGui/ScenePanel.h"

void WillEngine::EngineGui::ScenePanel::update(ImTextureID image, VkExtent2D& sceneSize, bool& sceneExtentChanged)
{
	ImGui::Begin("Scene");

	ImVec2 extent(ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

	// Update scene size if changed
	if (sceneSize.width != extent.x || sceneSize.height != extent.y)
	{
		if (holdCount > threshold)
		{
			sceneSize.width = extent.x;
			sceneSize.height = extent.y;
			sceneExtentChanged = true;
			holdCount = 0;
		}
		else
		{
			holdCount++;
		}
	}

	ImGui::Image(image, extent);

	ImGui::End();
}