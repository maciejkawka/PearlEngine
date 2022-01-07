#include"Editor/Core/EditorContext.h"
#include"Engine/Core/Utils/Logger.h"

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
