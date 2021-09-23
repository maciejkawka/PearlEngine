#include "Editor/Core/Editor.h"
#include"Editor/Core/EditorContext.h"
#include"Core/Utils/Logger.h"
#include"Core/Events/EventManager.h"
#include"Core/Events/TestEvent.h"

using namespace PrEditor::Core;

Editor::Editor()
{
	m_appContext = new EditorContext();

	PrCore::Events::EventListener listener;
	listener.connect<&Editor::OnEvent>(this);
	PrCore::Events::TestEvent e;
	listener(&e);
	PrCore::Events::EventManager::GetInstance()->AddListener(listener, PrCore::Events::TestEvent::s_type);
}

Editor::~Editor()
{
	delete m_appContext;
}
void Editor::PreFrame()
{
	PrCore::Events::TestEvent e;
	e.GetType();
	PrCore::Events::TestEvent1 e2;
	e2.GetType();
	e.GetType();
	PRLOG_INFO(e.GetType())
	PrCore::Events::TestEvent* e1 = new PrCore::Events::TestEvent();
	PrCore::Events::EventManager::GetInstance()->FireEvent(&e);
}

void Editor::OnFrame()
{
	
}

void Editor::PostFrame()
{

}

void Editor::OnEvent(PrCore::Events::Event* e)
{
	auto testEvent = dynamic_cast<PrCore::Events::TestEvent*>(e);
	if (e == nullptr)
		return;
	testEvent->Print();
	testEvent->Add(1, 2);
}


