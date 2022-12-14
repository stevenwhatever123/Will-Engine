#pragma once
#include <typeindex>

#include "Core/ECS/ComponentType.h"

namespace WillEngine
{
	class Component;

	class Entity
	{
	public:
		bool isEnable;

		std::string name;

		std::map <std::type_index, Component*> components;

		// For node hierarchy
		Entity* parent;
		std::vector<Entity*> children;

	public:

		Entity();
		Entity(const char* name);
		~Entity();

		void setName(const char* name);

	public:
		
		// Templates
		template<typename T> void addComponent(T* comp);

		template<class T> inline T* GetComponent()
		{
			return dynamic_cast<T*>(components[typeid(T)]);
		}

		template<class T> inline bool HasComponent()
		{
			return components[typeid(T)] != nullptr;
		}

		template<class T> inline void removeComponent()
		{
			std::map <std::type_index, Component*>::iterator it;

			it = components.find(typeid(T));

			delete components[typeid(T)];

			components.erase(it);
		}
	};
}