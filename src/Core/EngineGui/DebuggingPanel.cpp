#include "pch.h"
#include "Core/EngineGui/DebuggingPanel.h"

void WillEngine::EngineGui::DebuggingPanel::update(GameState* gameState, VulkanFramebuffer& attachments)
{
	ImGui::Begin("Rendering Debugger");

	ImGui::Checkbox("Enable Bloom", &gameState->gameSettings.enableBloom);

	if (ImGui::TreeNode("Bloom Viewer"))
	{
		static i32 mipLevel = 0;
		ImGui::SliderInt("Mip Level", &mipLevel, 0, gameState->graphicsState.upSampledImageDescriptorSetOutput.size() - 1);

		ImGui::Text("Downscale");
		// Since the last downscaled image is in the last image of upSampled
		// We use the last image of upSampled if it is outside the range of downSampled
		if(mipLevel > gameState->graphicsState.downSampledImageDescriptorSetOutput.size() - 1)
			ImGui::Image((ImTextureID)(ImTextureID)gameState->graphicsState.upSampledImage_ImGui[mipLevel], ImVec2(352, 240));
		else
			ImGui::Image((ImTextureID)gameState->graphicsState.downSampledImage_ImGui[mipLevel], ImVec2(352, 240));

		ImGui::Text("Upscale");
		ImGui::Image((ImTextureID)gameState->graphicsState.upSampledImage_ImGui[mipLevel], ImVec2(352, 240));

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("GBuffer Viewer"))
	{
		ImGui::Text("GBuffer0");
		ImGui::Image((ImTextureID)attachments.GBuffer0.imguiTextureDescriptorSet, ImVec2(352, 240));

		ImGui::Text("GBuffer1");
		ImGui::Image((ImTextureID)attachments.GBuffer1.imguiTextureDescriptorSet, ImVec2(352, 240));

		ImGui::Text("GBuffer2");
		ImGui::Image((ImTextureID)attachments.GBuffer2.imguiTextureDescriptorSet, ImVec2(352, 240));

		ImGui::Text("GBuffer3");
		ImGui::Image((ImTextureID)attachments.GBuffer3.imguiTextureDescriptorSet, ImVec2(352, 240));

		ImGui::TreePop();
	}

	ImGui::End();
}
