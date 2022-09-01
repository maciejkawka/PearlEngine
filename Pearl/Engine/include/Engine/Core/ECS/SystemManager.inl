#pragma once
#include "SystemManager.h"
#include <type_traits>

namespace PrCore::ECS {

	template<class System>
	void SystemManager::RegisterSystem()
	{
		PR_ASSERT(m_systemTypeCounter < MAX_SYSTEMS, "Cannot register more systems");

		if(!std::is_base_of<BaseSystem, System>::value)
			PR_ASSERT(false, "System must expand PrCore::ECS::BaseSystem");

		auto systemID = GetSystemID<System>();
		auto system = new System();
		system->Init_Base(m_entityManager);

		m_systems[systemID] = system;

		auto updateGroup = system->m_updateGroup;
		if (m_systemGroups.find(updateGroup) == m_systemGroups.end())
		{
			auto pair = std::make_pair(updateGroup, std::vector<BaseSystem*>());
			m_systemGroups.insert(pair);
		}
		
		m_systemGroups[updateGroup].push_back(system);

	}

	template<class System>
	void SystemManager::UpdateSystem(float p_dt)
	{
		auto systemID = GetSystemID<System>();
		if (m_systems[systemID] != nullptr)
			m_systems[systemID]->OnUpdate(p_dt);
		
	}

	template<class System>
	size_t SystemManager::GetSystemID()
	{
		static size_t s_SystemID = m_systemTypeCounter++;
		return s_SystemID;
	}

}
