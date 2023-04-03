#pragma once
#include "Core/ECS/Component.h"

#include "Core/Animation.h"
#include "Core/ECS/AnimationComponent.h"

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

		// A copy of world transformation
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

		// getLocalTransformation will rebuild the local transform whenever it's called
		mat4 getLocalTransformation() const;
		// getGlobalTransformation is the slowest way to get the global world transformation
		// It is calculated by transvering the tree back to the root entity
		// Usually called for updating worldTransformation
		mat4 getGlobalTransformation() const;
		// getWorldTransformation is faster than getGlobalTransformation for getting the global world transformation
		mat4& const getWorldTransformation() { return worldTransformation; };

		mat4 getLocalTransformation(const Animation* animation, const AnimationComponent* animationComp) const;
		mat4 getGlobalTransformation(const Animation* animation, const AnimationComponent* animationComp) const;

		void updateWorldTransformation() { worldTransformation = getGlobalTransformation(); };
		void updateWorldTransformation(const Animation* animation, const AnimationComponent* animationComp) { worldTransformation = getGlobalTransformation(animation, animationComp); };

		void updateAllChildWorldTransformation(const std::unordered_map<std::string, bool>* necessityMap = nullptr);
		void updateAllChildWorldTransformation(const Animation* animation, const AnimationComponent* animationComp, const std::unordered_map<std::string, bool>* necessityMap = nullptr);

	private:

	};
}