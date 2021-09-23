#include "Core/Entry/AppContext.h"
#include"Core/Utils/Logger.h"

#include"Core/Events/EventManager.h"

PrCore::Entry::AppContext::AppContext()
{
	PrCore::Utils::Logger::Init();
	PRLOG_INFO("Building AppContext");

	PrCore::Events::EventManager::Init();
}

PrCore::Entry::AppContext::~AppContext()
{
	PRLOG_INFO("Deleting AppContext")
}
