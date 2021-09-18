#pragma once
#include"Application.h"

PrCore::Entry::Application* CreateApplication();


int main(int argc, char** argv)
{
	//Create app outside the engine
	auto app = CreateApplication();

	//Run app
	app->Run();

	//Close and delete app
	delete app;

	return 0;
}