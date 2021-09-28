#include "Editor/Core/EditorContext.h"
#include"Core/Utils/Logger.h"

using namespace PrEditor::Core;
EditorContext::EditorContext():
	AppContext()
{
	
	PRLOG_INFO("Building EditorContext");
}

EditorContext::~EditorContext()
{
	PRLOG_INFO("Deleting EditorContext");
}
