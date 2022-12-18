#pragma once
#include "Core/ECS/Entity.h"

namespace WillEngine
{
	class Component
	{
	protected:

		Entity* parent;

	public:

		Component();
		Component(Entity* entity);
		virtual ~Component();

		virtual void update() {};

		void setParent(Entity* entity) { parent = entity; };

		virtual ComponentType getType() { return ComponentType::NullType; };

		template<class T> inline T* GetComponent()
		{
			return dynamic_cast<T*>(this);
		}
	};
}