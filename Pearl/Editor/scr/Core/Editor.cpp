#include "Editor/Core/Editor.h"
#include"Editor/Core/EditorContext.h"
#include"Core/Utils/Logger.h"
#include"Core/Events/EventManager.h"

using namespace PrEditor::Core;
using namespace PrCore::Events;

Editor::Editor()
{
	m_appContext = new EditorContext();
}

Editor::~Editor()
{
	delete m_appContext;
}
void Editor::PreFrame()
{
	m_appContext->m_window->PollEvents();
}

void Editor::OnFrame()
{
	if(PrCore::Input::InputManager::IsButtonHold(PrCore::Input::PrMouseButton::BUTTON_LEFT))
		PRLOG_INFO("Works B");

	if (PrCore::Input::InputManager::IsButtonReleased(PrCore::Input::PrMouseButton::BUTTON_MIDDLE))
		PRLOG_INFO("Works 4");

	if (PrCore::Input::InputManager::IsAnyKeyPressed())
		PRLOG_INFO("ANY PRESSED");
}

void Editor::PostFrame()
{
	m_appContext->m_window->SwapBuffers();
	m_appContext->m_input->ResetFlag();
	
	EventManager::GetInstance().Update();
}


