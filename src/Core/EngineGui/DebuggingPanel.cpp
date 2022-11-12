#include "pch.h"
#include "Core/EngineGui/DebuggingPanel.h"

void WillEngine::EngineGui::DebuggingPanel::update(VulkanFramebuffer& attachments)
{
	ImGui::Begin("Rendering Debugger");

	ImGui::Text("GBuffer0");
	ImGui::Image((ImTextureID)attachments.GBuffer0.imguiTextureDescriptorSet, ImVec2(352, 240));

	ImGui::Text("GBuffer1");
	ImGui::Image((ImTextureID)attachments.GBuffer1.imguiTextureDescriptorSet, ImVec2(352, 240));

	ImGui::Text("GBuffer2");
	ImGui::Image((ImTextureID)attachments.GBuffer2.imguiTextureDescriptorSet, ImVec2(352, 240));

	ImGui::Text("GBuffer3");
	ImGui::Image((ImTextureID)attachments.GBuffer3.imguiTextureDescriptorSet, ImVec2(352, 240));

	ImGui::Text("GBuffer4");
	ImGui::Image((ImTextureID)attachments.GBuffer4.imguiTextureDescriptorSet, ImVec2(352, 240));

	ImGui::End();
}
