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

}

void Editor::PostFrame()
{
	m_appContext->m_window->SwapBuffers();
	EventManager::GetInstance().Update();
}


