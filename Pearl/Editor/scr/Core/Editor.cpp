#include "Editor/Core/Editor.h"
#include"Editor/Core/EditorContext.h"
#include"Core/Utils/Logger.h"
#include"Core/Events/EventManager.h"
#include"Core/Events/TestEvent.h"

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
}

void Editor::OnFrame()
{
	EventManager::GetInstance().Update();
}

void Editor::PostFrame()
{

}


