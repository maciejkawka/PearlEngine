#pragma once
#include"Event.h"

#include"entt.hpp"
#include<map>
#include<list>

#define EVENTQUEUE_NUM 2

namespace PrCore::Events {

	typedef entt::delegate<void(EventPtr)> EventListener;

	class EventManager {

		typedef std::list<EventListener> EventListenerList;
		typedef std::map<EventType, EventListenerList> EventMap;
		typedef std::list<EventPtr> EventQueue;

	public:
		EventManager(EventManager&) = delete;
		EventManager(EventManager&&) = delete;
		EventManager& operator=(const EventManager&) = delete;
		EventManager& operator=(EventManager&&) = delete;

		inline static EventManager& GetInstance() { return *s_instance; }

		static void Init();
		static void Terminate();

		bool AddListener(const EventListener& p_listener, EventType p_type);
		bool RemoveListener(const EventListener& p_listener, EventType p_type);

		bool FireEvent(EventPtr& p_event);
		bool QueueEvent(EventPtr& p_event);

		void Update();

	private:
		EventManager();
		~EventManager() {}

		EventMap m_eventMap;
		EventQueue m_eventQueue[EVENTQUEUE_NUM];
		int m_activeQueue;

		static EventManager* s_instance;
	};
}