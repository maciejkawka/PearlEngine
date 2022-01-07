#include"Editor/Core/Editor.h"
#include"Engine/Core/Entry/EntryPoint.h"


PrCore::Entry::Application* CreateApplication()
{
	return new PrEditor::Core::Editor();
}