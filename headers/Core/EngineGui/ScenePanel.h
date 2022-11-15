#pragma once

namespace WillEngine::EngineGui::ScenePanel
{
	static u32 holdCount = 0;
	const static u32 threshold = 50;
	void update(ImTextureID image, VkExtent2D& sceneSize, bool& sceneExtentChanged);
}