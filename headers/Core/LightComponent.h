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
		f32 range;

		// For Point Light Shadow mapping
		mat4 matrices[6];

		LightUniform lightUniform;

	public:

		LightComponent();
		LightComponent(Entity* entity);
		LightComponent(Entity* entity, vec3 position);
		LightComponent(Entity* entity, vec3 position, vec4 color);
		virtual ~LightComponent();

		virtual void update();

		virtual void updateLightUniform();

		virtual void updateLightPosition(vec3 position);

		virtual ComponentType getType() { return id; };

		// Command call
		void shadowRendered();

		// Getters
		bool shouldRenderShadow() const;

	private:
	};
}