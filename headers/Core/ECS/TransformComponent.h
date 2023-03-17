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

	private:

		// A boolean to check if we have to update the world transformation
		mat4 worldTransformation;

	public:

		TransformComponent();
		TransformComponent(const vec3 position, const vec3 rotation, const vec3 scale);
		TransformComponent(Entity* entity);
		TransformComponent(Entity* entity, const vec3 position, const vec3 rotation, const vec3 scale);
		virtual ~TransformComponent();

		virtual void update() {};

		vec3& getPosition() { return position; };
		vec3& getRotation() { return rotation; };
		vec3& getScale() { return scale; };

		virtual ComponentType getType() { return id; };

		mat4 getLocalTransformation() const;
		mat4 getGlobalTransformation() const;
		mat4& const getWorldTransformation() { return worldTransformation; };

		void updateWorldTransformation() { worldTransformation = getGlobalTransformation(); };
		void updateAllChildWorldTransformation();

	private:

	};
}