#include "Core/Common/pearl_pch.h"
#include "Core/ECS/SystemManager.h"
#include"Core/ECS/SystemMap.h"

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
	{
		if(system->IsActive())
			system->OnUpdate(p_dt);
	}
}

void SystemManager::UpdateGroup(ECS::UpdateGroup p_systemGroup, float p_dt)
{
	UpdateGroup((uint8_t)p_systemGroup, p_dt);
}

void SystemManager::OnSerialize(Utils::JSON::json& p_serialized)
{
	for(int i=0;i< m_systemTypeCounter; i++)
	{
		Utils::JSON::json system;
		auto systemPtr = m_systems[i];
		system["systemType"] = typeid(*systemPtr).name();
		system["isActive"] = systemPtr->m_isActive;

		systemPtr->OnSerialize(system);

		p_serialized.push_back(system);
	}
}

void SystemManager::OnDeserialize(const Utils::JSON::json& p_deserialized)
{
	for (auto& systemJSON : p_deserialized)
	{
		std::string systemType = systemJSON["systemType"];
		DeduceSystemTypeByString(this, systemType);
	}

	for(int i=0;i< m_systemTypeCounter + 1; i++)
	{
		auto system = m_systems[i];
		auto& systemJSON = p_deserialized[i];

		system->SetActive(systemJSON["isActive"]);
		system->OnDeserialize(systemJSON);
	}
}
