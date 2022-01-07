#include"Editor/Core/Editor.h"
#include"Editor/Core/EditorContext.h"
#include"Engine/Core/Utils/Logger.h"
#include"Engine/Core/Events/EventManager.h"

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

void Editor::OnFrame(float p_deltaTime)
{
	if (PrCore::Input::InputManager::IsKeyHold(PrCore::Input::PrKey::LEFT_CONTROL))
	{
		auto mousePos = PrCore::Input::InputManager::GetMousePosition();
		auto xPos = mousePos.x;
		auto yPos = mousePos.y;

		PRLOG_INFO("Mouse Pos x: {0}, y: {1}", xPos, yPos);
	}

	if (PrCore::Input::InputManager::IsAnyKeyPressed())
		PRLOG_INFO("ANY PRESSED");

	//Show FPS
	if (PrCore::Input::InputManager::IsKeyHold(PrCore::Input::PrKey::LEFT_SHIFT))
		PRLOG_INFO("{0}", 1/p_deltaTime);
}

void Editor::PostFrame()
{
	m_appContext->m_window->SwapBuffers();
	m_appContext->m_input->ResetFlags();
	
	EventManager::GetInstance().Update();
}


