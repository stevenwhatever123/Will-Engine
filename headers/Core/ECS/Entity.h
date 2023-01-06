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

		// A valid id must be larger than 0, i.e. starting from 1
		const u32 id;

		std::string name;

		std::map <std::type_index, Component*> components;

		// For node hierarchy
		Entity* parent;
		std::vector<Entity*> children;

	public:

		Entity();
		Entity(const char* name);
		Entity(Entity* parent, const char* name);
		~Entity();

		void setName(const char* name);

		void addChild(Entity* child);

		bool hasChildren() const { return children.size() > 0; };
		u32 getChildrenSize() const { return children.size(); };

	private:

		// Used for generating an id for an entity
		static u32 idCounter;

	public:
		
		// Templates
		// Add component by object
		template<typename T> void addComponent(T* comp);

		// Add component by class
		template<class T> inline void addComponent();

		// Add component by component type
		void addComponent(ComponentType type);

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