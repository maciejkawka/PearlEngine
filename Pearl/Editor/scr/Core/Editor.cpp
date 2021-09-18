#include "Editor/Core/Editor.h"
#include"Editor/Core/EditorContext.h"

using namespace PrEditor::Core;

Editor::Editor()
{
	m_appContext = new EditorContext();
}

Editor::~Editor()
{

}
void Editor::PreFrame()
{
}

void Editor::OnFrame()
{
	
}

void Editor::PostFrame()
{
}


PrCore::Entry::Application* PrCore::Entry::InstansiateApplication()
{
	return new Editor();
}
