#include "Editor/Core/EditorContext.h"
#include"Core/Entry/EntryPoint.h"
#include"Core/Utils/Logger.h"

using namespace PrEditor::Core;
EditorContext::EditorContext():
	AppContext()
{
	PrCore::Utils::Logger::Init();
}

EditorContext::~EditorContext()
{
}
