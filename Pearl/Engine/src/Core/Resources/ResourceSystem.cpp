#include "Core/Common/pearl_pch.h"

#include "Core/Resources/ResourceSystem.h"

using namespace PrCore::Resources;

void ResourceSystem::UnregisterDatabasesAll()
{
	m_resourceDatabaseTypes.clear();
}

ResourceSystem::~ResourceSystem()
{
	UnregisterDatabasesAll();
}
