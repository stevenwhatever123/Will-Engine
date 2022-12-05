#include "pch.h"
#include "Core/EngineGui/LightPanel.h"

void WillEngine::EngineGui::LightPanel::update(std::vector<Light*>& lights)
{
	if (lights.size() > 0)
	{
		ImGui::Begin("Light Control");

		for (u32 i = 0; i < lights.size(); i++)
		{
			ImGui::PushID(i);

			ImGui::Text("Light %u", i);

			ImGui::DragFloat3("", &lights[i]->position.x, 0.1f);

			ImGui::DragFloat("Intensity", &lights[i]->lightUniform.intensity, 0.1f, 0, 100);

			ImGui::Checkbox("Draw Gui", &lights[i]->renderGui);

			ImGui::PopID();
		}

		ImGui::End();
	}
}