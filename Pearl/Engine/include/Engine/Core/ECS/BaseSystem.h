#pragma once
#include"Core/ECS/EntityViewer.h"
#include"Core/Utils/ISerializable.h"


namespace PrCore::ECS {

	enum class UpdateGroup : uint8_t
	{
		Custom = 0,
		Update = 1<<3,
		FixUpdate = 1<<4,
		LateUpdate = 1<<7
	};

	class BaseSystem: public Utils::ISerializable {
	public:
		BaseSystem() :
			m_entityViewer(nullptr),
			m_updateGroup((uint8_t)UpdateGroup::Update),
			m_isActive(true)
		{}

		~BaseSystem() = default;

		virtual void OnUpdate(float p_dt) = 0;
		virtual void OnCreate() {}
		virtual void OnEnable() {}
		virtual void OnDisable() {}

		inline void SetActive(bool p_isActive)
		{
			if (m_isActive && p_isActive != m_isActive)
				OnDisable();
			else if (!m_isActive && p_isActive != m_isActive)
				OnEnable();

			m_isActive = p_isActive;
		}

		inline bool IsActive() const { return m_isActive; }

		inline virtual void OnSerialize(Utils::JSON::json& p_serialized) override {}
		inline virtual void OnDeserialize(const Utils::JSON::json& p_deserialized) override {}

	protected:
		EntityViewer m_entityViewer;
		uint8_t m_updateGroup;
		bool m_isActive;

	private:
		void Init(EntityManager* p_entityManager)
		{
			m_entityViewer = EntityViewer(p_entityManager);
			m_updateGroup = (uint8_t)UpdateGroup::Update;
			m_isActive = true;
		}

		friend class SystemManager;
	};
}