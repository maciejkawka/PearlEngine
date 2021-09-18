#include"Editor/Core/Editor.h"
#include"Core/Entry/EntryPoint.h"


PrCore::Entry::Application* CreateApplication()
{
	return new PrEditor::Core::Editor();
}