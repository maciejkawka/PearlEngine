#include "Core/Common/pearl_pch.h"
#include "Core/ECS/SystemManager.h"

using namespace PrCore::ECS;

SystemManager::SystemManager(EntityManager* p_entityManager):
m_entityManager(p_entityManager),
m_systemTypeCounter(0)
{}

SystemManager::~SystemManager()
{
	for (auto system : m_systems)
		delete system;
}

void SystemManager::Reset()
{
	for (auto system : m_systems)
		delete system;

	m_systemTypeCounter = 0;
	m_systemGroups.clear();
}

void SystemManager::UpdateGroup(uint8_t p_systemGroup, float p_dt)
{
	auto systemsIterator = m_systemGroups.find(p_systemGroup);
	if(systemsIterator == m_systemGroups.end())
		return;

	auto systemsVector = *systemsIterator;
	for (auto system : systemsVector.second)
		system->OnUpdate(p_dt);
}
