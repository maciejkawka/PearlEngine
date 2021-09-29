#pragma once
#include"Application.h"

PrCore::Entry::Application* CreateApplication();


int main(int argc, char** argv)
{
	//Memory Leaks Check
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	//Create app outside the engine
	auto app = CreateApplication();

	//Run app
	app->Run();

	//Close and delete app
	delete app;

	return 0;
}