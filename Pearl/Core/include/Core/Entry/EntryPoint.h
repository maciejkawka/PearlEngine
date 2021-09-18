#pragma once
#include"Application.h"

PrCore::Entry::Application* PrCore::Entry::InstansiateApplication();


int main(int argc, char** argv)
{
	//Create app outside the engine
	auto app = PrCore::Entry::InstansiateApplication();

	//Run app
	app->Run();

	//Close and delete app
	delete app;

	return 0;
}