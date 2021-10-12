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
	if (PrCore::Input::InputManager::IsKeyHold(PrCore::Input::PrKey::LEFT_CONTROL))
	{
		auto xPos = PrCore::Input::InputManager::GetMouseX();
		auto yPos = PrCore::Input::InputManager::GetMouseY();
		PRLOG_INFO("Mouse Pos x: {0}, y: {1}", xPos, yPos);
	}

	if (PrCore::Input::InputManager::IsButtonReleased(PrCore::Input::PrMouseButton::BUTTON_MIDDLE))
		PRLOG_INFO("Works 4");

	if (PrCore::Input::InputManager::IsAnyKeyPressed())
		PRLOG_INFO("ANY PRESSED");
}

void Editor::PostFrame()
{
	m_appContext->m_window->SwapBuffers();
	m_appContext->m_input->ResetFlags();
	
	EventManager::GetInstance().Update();
}


