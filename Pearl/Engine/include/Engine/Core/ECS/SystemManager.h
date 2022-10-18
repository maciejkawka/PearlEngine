#pragma once
#include "Core/ECS/BaseSystem.h"

#include "Core/Utils/NonCopyable.h"
#include "Core/Utils/ISerializable.h"

#include <unordered_map>
#include <array>

namespace PrCore::ECS {

	class EntityManager;

	constexpr unsigned int MAX_SYSTEMS = 100;

	class SystemManager: public Utils::NonCopyable, Utils::ISerializable  {
	public:
		SystemManager() = delete;
		SystemManager(EntityManager* p_entityManager);

		~SystemManager();

		template<class System>
		void RegisterSystem();

		void Reset();

		//Event functions
		void UpdateOnEnable();

		void UpdateOnDisable();

		void UpdateGroup(uint8_t p_systemGroup, float p_dt);

		void UpdateGroup(ECS::UpdateGroup p_systemGroup, float p_dt);

		template<class System>
		void UpdateSystem(float p_dt);

		template<class System>
		void SetActiveSystem(bool p_isActive);

		template<class System>
		bool IsActiveSystem();

		void OnSerialize(Utils::JSON::json& p_serialized) override;

		void OnDeserialize(const Utils::JSON::json& p_deserialized) override;

	private:
		template<class System>
		size_t GetSystemID();

		size_t m_systemTypeCounter;

		//map holding system to updated per group
		std::unordered_map<uint8_t, std::vector<BaseSystem*>> m_systemGroups;

		//vector holds systems to update per system
		std::array<BaseSystem*, MAX_SYSTEMS> m_systems;

		//queues to update event functions
		std::queue<BaseSystem*> m_onEnable;
		std::queue<BaseSystem*> m_onDisable;

		EntityManager* m_entityManager;
	};
}

#include "Core/ECS/SystemManager.inl"