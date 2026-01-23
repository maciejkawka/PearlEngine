#include "Core/Common/pearl_pch.h"

#include "Core/Entry/Application.h"
#include "Core/Entry/AppContext.h"
#include "Core/Events/WindowEvents.h"
#include "Core/Events/EventManager.h"
#include "Core/Utils/Clock.h"

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
			OnFrame(PrSystems::Get<Utils::Clock>()->GetUnscaledDeltaTime());
		PostFrame();

		PrSystems::Get<Utils::Clock>()->Tick();
	}
}

void Application::SubscribeEvents()
{
	EventListener windowCloseListener;
	windowCloseListener.Connect<&Application::OnWindowClose>(this);
	PrSystems::Get<EventManager>()->AddListener(windowCloseListener, PrCore::WindowCloseEvent::s_type);

	EventListener windowMinimalizedListener;
	windowMinimalizedListener.Connect<&Application::OnWindowMinimalized>(this);
	PrSystems::Get<EventManager>()->AddListener(windowMinimalizedListener, PrCore::WindowMinimalizeEvent::s_type);
}

void Application::OnWindowClose(EventPtr p_event)
{
	m_shouldClose = true;
}

void Application::OnWindowMinimalized(EventPtr p_event)
{
	auto minimalizeEvent = std::static_pointer_cast<WindowMinimalizeEvent>(p_event);
	m_minimalized = minimalizeEvent->m_minimalized;
}
