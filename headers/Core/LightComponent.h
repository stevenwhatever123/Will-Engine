#pragma once
#include "Light.h"
#include "Core/ECS/Component.h"
#include "Core/UniformClass.h"

namespace WillEngine
{
	class LightComponent : public Component
	{
	public:

		static const ComponentType id = ComponentType::LightType;

		const u32 lightIndex;

	public:

		LightComponent();
		LightComponent(const Light* light);
		virtual ~LightComponent();

		virtual ComponentType getType() { return id; };

	private:
	};
}