#include "Editor/Core/EditorContext.h"

#include "Engine/Core/Entry/EngineCore.h"

int main(int argc, char** argv)
{
	//Memory Leaks Check
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	PrCore::EngineCoreParams params;
	auto engineCore = PrCore::CreateEngineCore(std::make_unique<PrEditor::EditorContext>());
	if (!engineCore->Initalize(params))
		return 1;

	while (engineCore->Run());

	engineCore->Terminate();

	return 0;
}