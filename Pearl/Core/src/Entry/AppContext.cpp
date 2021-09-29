#include"Core/Common/pearl_pch.h"

#include "Core/Entry/AppContext.h"
#include"Core/Events/EventManager.h"
#include"Core/Utils/Logger.h"

PrCore::Entry::AppContext::AppContext()
{
	PrCore::Utils::Logger::Init();
	PRLOG_INFO("Building AppContext");

	//Init Engine Subsystems
	PrCore::Events::EventManager::Init();
}

PrCore::Entry::AppContext::~AppContext()
{
	PRLOG_INFO("Deleting AppContext");

	PrCore::Events::EventManager::Terminate();
}
