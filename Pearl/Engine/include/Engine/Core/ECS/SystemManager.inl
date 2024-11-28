#pragma once
#include "SystemManager.h"
#include <type_traits>

namespace PrCore::ECS {

	template<class System>
	void SystemManager::RegisterSystem()
	{
		static_assert(std::is_base_of<BaseSystem, System>::value, "System must expand PrCore::ECS::BaseSystem");

		PR_ASSERT(m_systemTypeCounter < MAX_SYSTEMS, "Cannot register more systems");

		auto systemID = GetSystemID<System>();
		if (m_systems[systemID] != nullptr)
		{
			PR_ASSERT(false, "System already registered");
			return;
		}

		auto system = new System();
		system->Init(m_entityManager);
		system->OnCreate();
		m_onEnable.push(system);

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
		static_assert(std::is_base_of<BaseSystem, System>::value, "System must expand PrCore::ECS::BaseSystem");

		auto systemID = GetSystemID<System>();
		if (m_systems[systemID] != nullptr)
			m_systems[systemID]->OnUpdate(p_dt);
		
	}

	template<class System>
	void SystemManager::SetActiveSystem(bool p_isActive)
	{
		static_assert(std::is_base_of<BaseSystem, System>::value, "System must expand PrCore::ECS::BaseSystem");

		auto systemID = GetSystemID<System>();
		auto system = m_systems[systemID];
		if (system != nullptr)
		{
			auto actualIsActive = system->m_isActive;
			if (actualIsActive && p_isActive != actualIsActive)
				m_onDisable.push(system);
			else if (!actualIsActive && p_isActive != actualIsActive)
				m_onEnable.push(system);

			m_systems[systemID]->SetActive(p_isActive);
		}
	}

	template<class System>
	bool SystemManager::IsActiveSystem() const
	{
		static_assert(std::is_base_of<BaseSystem, System>::value, "System must expand PrCore::ECS::BaseSystem");

		auto systemID = GetSystemID<System>();
		return m_systems[systemID]->IsActive();
	}

	template<class System>
	size_t SystemManager::GetSystemID()
	{
		static_assert(std::is_base_of<BaseSystem, System>::value, "System must expand PrCore::ECS::BaseSystem");
		static size_t s_SystemID = m_systemTypeCounter++;
		return s_SystemID;
	}

}
