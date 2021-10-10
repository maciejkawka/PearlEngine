#include"Core/Common/pearl_pch.h"

#include "Core/Entry/Application.h"
#include"Core/Entry/AppContext.h"
#include"Core/Utils/Logger.h"

using namespace PrCore::Entry;

Application::Application():
	m_appContext(nullptr)
{
}

Application::~Application()
{
}

void Application::Run()
{
	//Init Time

	while (IsRunning())
	{
		if (IsMinimalised()) continue;

		PreFrame();
		OnFrame(/*pass deltaTime*/);
		PostFrame();

		//Update Time
	}
}

bool Application::IsRunning()
{
	return !m_appContext->m_window->ShouldClose();
}

bool Application::IsMinimalised()
{
	return false;
}
