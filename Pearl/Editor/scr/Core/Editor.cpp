#include "Editor/Core/Editor.h"
#include"Editor/Core/EditorContext.h"
#include"Core/Utils/Logger.h"
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
	PRLOG_ERROR("TEST ERROR");
}

void Editor::OnFrame()
{
	
}

void Editor::PostFrame()
{

}
