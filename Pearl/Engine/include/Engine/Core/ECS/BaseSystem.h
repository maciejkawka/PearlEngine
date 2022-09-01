#pragma once
#include"Core/ECS/EntityViewer.h"


namespace PrCore::ECS {

	enum class UpdateGroup
	{
		Update = 1<<3,
		FixUpdate = 1<<4,
		LateUpdate = 1<<7
	};

	class BaseSystem {
	public:
		BaseSystem() :
			m_entityViewer(nullptr),
			m_updateGroup((uint8_t)UpdateGroup::Update),
			m_isActive(true)
		{}

		~BaseSystem() = default;
		virtual void OnUpdate(float p_dt) = 0;
		virtual void Init() {}
	
	protected:
		EntityViewer m_entityViewer;
		uint8_t m_updateGroup;
		bool m_isActive;

	private:
		void Init_Base(EntityManager* p_entityManager)
		{
			m_entityViewer = EntityViewer(p_entityManager);
			m_updateGroup = (uint8_t)UpdateGroup::Update;
			m_isActive = true;

			//If the system need to inistiate something do it there
			Init();
		}

		friend class SystemManager;
	};
}