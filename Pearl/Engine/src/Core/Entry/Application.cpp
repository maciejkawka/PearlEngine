#include"Core/Common/pearl_pch.h"

#include "Core/Entry/Application.h"
#include"Core/Entry/AppContext.h"
#include"Core/Events/WindowEvents.h"
#include"Core/Events/EventManager.h"
#include"Core/Utils/Clock.h"

using namespace PrCore::Entry;

Application::Application():
	m_appContext(nullptr),
	m_shouldClose(false),
	m_minimalized(false)
{
}

void Application::Run()
{
	while (!m_shouldClose)
	{
		PreFrame();
		if (!m_minimalized)
			OnFrame(Utils::Clock::GetInstance().GetUnscaledDeltaTime());
		PostFrame();

		//PRLOG_INFO("{0}", gameClock.GetRealTime());
		Utils::Clock::GetInstance().Tick();
	}
}

void Application::SubscribeEvents()
{
	Events::EventListener windowCloseListener;
	windowCloseListener.connect<&Application::OnWindowClose>(this);
	Events::EventManager::GetInstance().AddListener(windowCloseListener, PrCore::Events::WindowCloseEvent::s_type);

	Events::EventListener windowMinimalizedListener;
	windowMinimalizedListener.connect<&Application::OnWindowMinimalized>(this);
	Events::EventManager::GetInstance().AddListener(windowMinimalizedListener, PrCore::Events::WindowMinimalizeEvent::s_type);
}

void Application::OnWindowClose(Events::EventPtr p_event)
{
	m_shouldClose = true;
}

void Application::OnWindowMinimalized(Events::EventPtr p_event)
{
	auto minimalizeEvent = std::static_pointer_cast<Events::WindowMinimalizeEvent>(p_event);
	m_minimalized = minimalizeEvent->m_minimalized;
}
