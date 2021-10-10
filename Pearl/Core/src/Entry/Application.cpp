#include"Core/Common/pearl_pch.h"

#include "Core/Entry/Application.h"
#include"Core/Entry/AppContext.h"
#include"Core/Utils/Logger.h"
#include"Core/Events/WindowEvents.h"
#include"Core/Events/EventManager.h"



using namespace PrCore::Entry;

Application::Application():
	m_appContext(nullptr),
	m_shouldClose(false),
	m_minimalized(false)
{
}

Application::~Application()
{
}

void Application::Run()
{
	//Init Time

	while (!m_shouldClose)
	{
		

		PreFrame();
		if (!m_minimalized) //If minimalized do not update game
			OnFrame(/*pass deltaTime*/);
		PostFrame();

		//Update Time
	}
}

void PrCore::Entry::Application::SubscribeEvents()
{
	PrCore::Events::EventListener windowCloseListener;
	windowCloseListener.connect<&Application::OnWindowClose>(this);
	PrCore::Events::EventManager::GetInstance().AddListener(windowCloseListener, PrCore::Events::WindowCloseEvent::s_type);

	PrCore::Events::EventListener windowMinimalizedListener;
	windowMinimalizedListener.connect<&Application::OnWindowMinimalized>(this);
	PrCore::Events::EventManager::GetInstance().AddListener(windowMinimalizedListener, PrCore::Events::WindowMinimalizeEvent::s_type);
}

void Application::OnWindowClose(PrCore::Events::EventPtr p_event)
{
	m_shouldClose = true;
}

void PrCore::Entry::Application::OnWindowMinimalized(PrCore::Events::EventPtr p_event)
{
	auto minimalizeEvent = std::static_pointer_cast<PrCore::Events::WindowMinimalizeEvent>(p_event);
	m_minimalized = minimalizeEvent->m_minimalized;
}
