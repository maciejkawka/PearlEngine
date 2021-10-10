#include"Core/Common/pearl_pch.h"


#include"Core/Events/EventManager.h"
#include"Core/Utils/Logger.h"

using namespace PrCore::Events;

EventManager* EventManager::s_instance = nullptr;

EventManager::EventManager() :
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

void EventManager::Terminate()
{
	if (s_instance)
	{
		delete s_instance;
		PRLOG_INFO("EventManager Terminate");
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
		for (auto it = eventListeners.cbegin(); it!= eventListeners.cend(); ++it)
		{
			if (p_listener == *it)
			{
				eventListeners.erase(it);
				if (eventListeners.empty())
					m_eventMap.erase(p_type);
				return true;
			}
		}
	}

	return false;
}

bool EventManager::FireEvent(EventPtr& p_event)
{
	bool sucess = false;

	auto findListener = m_eventMap.find(p_event->GetType());
	if (findListener != m_eventMap.end())
	{
		const auto& eventListenerList = findListener->second;
		for (EventListener listener : eventListenerList)
		{
			listener(p_event);
			sucess = true;
		}

	}

	return sucess;
}

bool EventManager::QueueEvent(EventPtr& p_event)
{
	auto findListener = m_eventMap.find(p_event->GetType());
	if (findListener != m_eventMap.end())
	{
		m_eventQueue[m_activeQueue].push_back(p_event);
		return true;
	}

	PRLOG_WARN("No listeners for event: {0}", p_event->GetType());
	return false;
}

void EventManager::Update()
{
	auto queueToProcess = m_activeQueue;
	m_activeQueue = (++m_activeQueue) % EVENTQUEUE_NUM;
	m_eventQueue[m_activeQueue].clear();

	while (!m_eventQueue[queueToProcess].empty())
	{
		auto eventToProcess = m_eventQueue[queueToProcess].front();
		m_eventQueue[queueToProcess].pop_front();

		auto eventType = eventToProcess->GetType();

		auto findListener = m_eventMap.find(eventType);
		if (findListener != m_eventMap.end())
		{
			auto& eventListeners = findListener->second;
			for (auto& listener : eventListeners)
				listener(eventToProcess);
		}
	}
}