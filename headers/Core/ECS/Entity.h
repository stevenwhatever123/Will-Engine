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

		Entity* getRoot();

		bool hasParent() const { return parent ?  true :  false; };
		Entity* getParent() const { return parent; };

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

		template<class T> inline bool AnyParentHasComponent()
		{
			Entity* parentEntity = this;

			while (parentEntity)
			{
				if (parentEntity->HasComponent<T>())
					return true;

				if (!parentEntity->hasParent())
					return false;

				parentEntity = parentEntity->getParent();
			}

			return false;
		}

		// This would return the FIRST component found when transversing the node hierarchy back to the root
		template<class T> inline T* AnyParentGetComponent()
		{
			Entity* parentEntity = this;

			while (parentEntity)
			{
				if (parentEntity->HasComponent<T>())
					return parentEntity->GetComponent<T>();

				if (!parentEntity->hasParent())
					return nullptr;

				parentEntity = parentEntity->getParent();
			}

			return nullptr;
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