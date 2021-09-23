#pragma once
#include"Event.h"
#include"fastDelegates/delegateV2.hpp"
#include<map>
#include<list>


namespace PrCore::Events {
	
#define EVENTQUEUE_NUM 2

	typedef entt::delegate<void(Event*)> EventListener;

	class EventManager {

		typedef std::list<EventListener> EventListenerList;
		typedef std::map<EventType, EventListenerList> EventMap;
		typedef std::list<Event*> EventQueue;

	public:
		~EventManager() {}

		inline static EventManager* GetInstance() { return s_instance; }

		static void Init();

		bool AddListener(const EventListener& p_listener,  EventType p_type);
		bool RemoveListener(const EventListener& p_listener, EventType p_type);

		bool FireEvent(Event* p_event);
		bool QueueEvent(Event& p_event);
		bool DequeueEvent(Event& p_event);

		void Update();

	private:
		EventManager();
		static EventManager* s_instance;

		EventMap m_eventMap;
		EventQueue m_eventQueue[EVENTQUEUE_NUM];
		int m_activeQueue;


	};
}