#include"Core/Events/EventManager.h"
#include"Core/Utils/Logger.h"

using namespace PrCore::Events;

EventManager* EventManager::s_instance = nullptr;

EventManager::EventManager():
	m_activeQueue(0)
{}

void EventManager::Init()
{
	if (!s_instance)
	{
		s_instance = new EventManager();
		PRLOG_INFO("EventManager Init");
	}
}

bool EventManager::AddListener(const EventListener& p_listener, EventType p_type)
{
	auto& eventListenerList = m_eventMap[p_type];

	for (auto& eventListener : eventListenerList)
	{
		if (p_listener == eventListener)
		{
			uint32_t type = p_type;
			PRLOG_WARN("Multiple listener in eventType: {0}", type);
			return false;
		}
	}

	eventListenerList.push_back(p_listener);
	return true;
}

bool EventManager::RemoveListener(const EventListener& p_listener, EventType p_type)
{
	auto findListener = m_eventMap.find(p_type);
	if (findListener != m_eventMap.end())
	{
		auto& eventListeners = findListener->second;
		for (auto& listener : eventListeners)
			return true;
	}

	return false;
}

bool EventManager::FireEvent(Event* p_event)
{
	bool sucess = false;

	auto findListener = m_eventMap.find(p_event->GetType());
	if (findListener != m_eventMap.end())
	{
		const auto& eventListenerList = findListener->second;
		for (EventListener listener : eventListenerList)
			listener(p_event);
		
		sucess = true;
	}

	return sucess;
}
		
