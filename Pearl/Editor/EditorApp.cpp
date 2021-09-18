#include"Editor/Core/Editor.h"
#include"Core/Entry/EntryPoint.h"


PrCore::Entry::Application* InstansiateApplication()
{
	return new PrEditor::Core::Editor();
}