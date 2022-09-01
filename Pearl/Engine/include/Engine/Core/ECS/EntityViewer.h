#pragma once
#include"Core/ECS/EntityManager.h"

namespace PrCore::ECS {
	
	class EntityViewer {
	public:
		EntityViewer() = delete;
		EntityViewer(EntityManager* p_entityManager):
		m_entityManager(p_entityManager)
		{}

		template<typename... ComponentType>
		EntityManager::TypedView<ComponentType...> EntitesWithComponents()
		{
			return m_entityManager->GetEntitiesWithComponents<ComponentType...>();
		}

		EntityManager::BasicView AllEntities()
		{
			return m_entityManager->GetAllEntities();
		}

	private:
		EntityManager* m_entityManager;
	};
}
