#pragma once

#include"Core/Events/Event.h"
#include"Core/ECS/EntityManager.h"

namespace PrCore::Events {

	class EntityCreatedEvent : public Event {
	public:
		EntityCreatedEvent(ECS::Entity p_entity) :
			m_entity(p_entity)
		{}

		ECS::Entity m_entity;

		virtual inline EventType GetType() { return s_type; }
		inline const static EventType s_type = 0x38ca239c;
	};

	class EntityDestroyedEvent : public Event {
	public:
		EntityDestroyedEvent(ECS::Entity p_entity) :
			m_entity(p_entity)
		{}

		ECS::Entity m_entity;

		virtual inline EventType GetType() { return s_type; }
		inline const static EventType s_type = 0x1153264b;
	};


}