#pragma once
#include "Core/ECS/Entity.h"

namespace WillEngine
{
	class Component
	{
	public:

	public:

		Component();
		virtual ~Component();

		virtual void update() {};

		template<class T> inline T* GetComponent()
		{
			return dynamic_cast<T*>(this);
		}
	};
}