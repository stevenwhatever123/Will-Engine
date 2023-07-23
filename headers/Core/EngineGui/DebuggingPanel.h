#pragma once

#include "Core/GameState.h"
#include "Core/Vulkan/VulkanDefines.h"

namespace WillEngine::EngineGui::DebuggingPanel
{
	void update(GameState* gameState, VulkanFramebuffer& attachments);
}