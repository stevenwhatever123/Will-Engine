#pragma once
#include "Core/ECS/Component.h"
#include "Core/UniformClass.h"

namespace WillEngine
{
	class LightComponent : public Component
	{
	public:

		static const ComponentType id = ComponentType::LightType;

		vec3 lastPosition;
		vec3 currentPosition;

		bool renderShadow;

		// For Point Light Shadow mapping
		mat4 matrices[6];

		LightUniform lightUniform;

	public:

		LightComponent();
		LightComponent(vec3 position);
		LightComponent(vec3 position, vec4 color);
		virtual ~LightComponent();

		virtual void update();

		virtual void updateLightUniform();

		virtual void updateLightPosition(vec3 position);

		// Command call
		void shadowRendered();

		// Getters
		bool shouldRenderShadow() const;

	private:
	};
}