#pragma once
#include "Core/ECS/BaseSystem.h"

#include "Core/Utils/NonCopyable.h"

#include <unordered_map>
#include <array>

namespace PrCore::ECS {

	class EntityManager;

	constexpr unsigned int MAX_SYSTEMS = 100;

	class SystemManager: Utils::NonCopyable {
	public:
		SystemManager() = delete;
		SystemManager(EntityManager* p_entityManager);

		~SystemManager();

		template<class System>
		void RegisterSystem();

		void Reset();

		void UpdateGroup(uint8_t p_systemGroup, float p_dt);

		template<class System>
		void UpdateSystem(float p_dt);

		template<class System>
		void SetActiveSystem(bool p_isActive);

		template<class System>
		bool IsActiveSystem();

	private:
		template<class System>
		size_t GetSystemID();

		size_t m_systemTypeCounter;

		//map holding system to updated per group
		std::unordered_map<uint8_t, std::vector<BaseSystem*>> m_systemGroups;

		//vector holds systems to update per system
		std::array<BaseSystem*, MAX_SYSTEMS> m_systems;

		EntityManager* m_entityManager;
	};
}

#include "Core/ECS/SystemManager.inl"