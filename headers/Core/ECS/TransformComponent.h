#pragma once
#include "Core/ECS/Component.h"

namespace WillEngine
{
	class TransformComponent : public Component
	{
	public:

		static const ComponentType id = ComponentType::TransformType;

		vec3 position;
		vec3 rotation;
		vec3 scale;

	public:

		TransformComponent();
		TransformComponent(Entity* entity);
		virtual ~TransformComponent();

		virtual void update() {};

		vec3& getPosition() { return position; };
		vec3& getRotation() { return rotation; };
		vec3& getScale() { return scale; };

		virtual ComponentType getType() { return id; };

		mat4 getLocalTransformation() const;
		mat4 getGlobalTransformation() const;

	private:

	};
}