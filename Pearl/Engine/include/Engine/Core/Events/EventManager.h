#pragma once
#include"Event.h"
#include"Core/Utils/Singleton.h"

#include"entt.hpp"
#include<map>
#include<list>

#define EVENTQUEUE_NUM 2

namespace PrCore::Events {

	typedef entt::delegate<void(EventPtr)> EventListener;

	class EventManager: public Utils::Singleton<EventManager> {

		typedef std::list<EventListener> EventListenerList;
		typedef std::map<EventType, EventListenerList> EventMap;
		typedef std::list<EventPtr> EventQueue;

	public:
		bool AddListener(const EventListener& p_listener, EventType p_type);
		bool RemoveListener(const EventListener& p_listener, EventType p_type);

		bool FireEvent(EventPtr& p_event);
		bool QueueEvent(EventPtr& p_event);

		void Update();

	private:
		EventManager();
		EventMap m_eventMap;
		EventQueue m_eventQueue[EVENTQUEUE_NUM];
		int m_activeQueue;

		friend Singleton<EventManager>;
	};
}