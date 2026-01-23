#pragma once

#include "Event.h"
#include "Delegate.h"

#include "Core/Utils/SystemProvider.h"

#include <map>
#include <list>

#define EVENTQUEUE_NUM 2

namespace PrCore {

	typedef Delegate<void(EventPtr)>       EventListener;

	class EventManager : public Utils::ISystem {
	public:
		EventManager();
		virtual ~EventManager() = default;

		bool AddListener(const EventListener& p_listener, EventType p_type);
		bool RemoveListener(const EventListener& p_listener, EventType p_type);

		bool FireEvent(EventPtr& p_event);
		bool QueueEvent(EventPtr& p_event);

		void Update();

	private:
		typedef std::list<EventListener> EventListenerList;
		typedef std::map<EventType, EventListenerList> EventMap;
		typedef std::list<EventPtr> EventQueue;

		EventMap m_eventMap;
		EventQueue m_eventQueue[EVENTQUEUE_NUM];
		int m_activeQueue;
	};
}