#pragma once

#include"Core/Events/Event.h"
#include"Core/ECS/EntityManager.h"
#include"Core/ECS/BaseComponent.h"

namespace PrCore::Events {

	class EntityCreatedEvent : public Event {
	public:
		EntityCreatedEvent(ECS::Entity p_entity) :
			m_entity(p_entity)
		{}

		ECS::Entity m_entity;

		DEFINE_EVENT_GUID(0x38ca239c);
	};

	class EntityDestroyedEvent : public Event {
	public:
		EntityDestroyedEvent(ECS::Entity p_entity) :
			m_entity(p_entity)
		{}

		ECS::Entity m_entity;

		DEFINE_EVENT_GUID(0x1153264b);
	};

	template<class Component>
	class ComponentAddedEvent : public Event {
	public:
		ComponentAddedEvent(ECS::Entity p_entity, Component* p_component) :
			m_entity(p_entity),
		m_component(p_component)
		{}

		ECS::Entity m_entity;
		Component* m_component;

		virtual inline EventType GetType() const { return s_type; }
		const static EventType s_type;
	};

	template<class Component>
	const EventType ComponentAddedEvent<Component>::s_type = typeid(ComponentAddedEvent<Component>).hash_code() + 0x8976fc84;

	template<class Component>
	class ComponentRemovedEvent : public Event {
	public:
		ComponentRemovedEvent(ECS::Entity p_entity, Component* p_component) :
			m_entity(p_entity),
			m_component(p_component)
		{}

		ECS::Entity m_entity;
		Component* m_component;

		virtual inline EventType GetType() const { return s_type; }
		const static EventType s_type;
	};

	template<class Component>
	const EventType ComponentRemovedEvent<Component>::s_type = typeid(ComponentRemovedEvent<Component>).hash_code() + 0x353e5392;

}