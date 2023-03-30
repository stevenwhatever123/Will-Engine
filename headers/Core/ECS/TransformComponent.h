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

		const vec3& const getPosition() { return position; };
		const vec3& const getRotation() { return rotation; };
		const vec3& const getScale() { return scale; };

		vec3& getModifiablePosition() { return position; };
		vec3& getModifiableRotation() { return rotation; };
		vec3& getModifiableScale() { return scale; };

		virtual ComponentType getType() { return id; };

		mat4 getLocalTransformation() const;
		// getGlobalTransformation is the slowest way to get the global world transformation
		// It is usually called for updating worldTransformation
		mat4 getGlobalTransformation() const;
		// getWorldTransformation is faster than getGlobalTransformation for getting the global world transformation
		mat4& const getWorldTransformation() { return worldTransformation; };

		void updateWorldTransformation() { worldTransformation = getGlobalTransformation(); };
		void updateAllChildWorldTransformation();

	private:

	};
}