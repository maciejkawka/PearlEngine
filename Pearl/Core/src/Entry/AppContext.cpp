#include"Core/Common/pearl_pch.h"

#include "Core/Entry/AppContext.h"
#include"Core/Events/EventManager.h"
#include"Core/Utils/Logger.h"


PrCore::Entry::AppContext::AppContext()
{
	PrCore::Utils::Logger::Init();
	PRLOG_INFO("Building AppContext");

	//Init Engine Subsystems
	PrCore::Events::EventManager::Init();

	PrCore::Windowing::WindowContext context;
	PrCore::Windowing::GLWindow::InitDevice(context);

	PrCore::Windowing::WindowSettings windowSettings;
	m_window = std::make_unique<PrCore::Windowing::GLWindow>(windowSettings);
	
	m_input = std::make_unique<PrCore::Input::InputManager>();
}

PrCore::Entry::AppContext::~AppContext()
{
	PRLOG_INFO("Deleting AppContext");

	PrCore::Windowing::GLWindow::TerminateDevice();
	PrCore::Events::EventManager::Terminate();
}
