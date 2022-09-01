#pragma once
#include"Core/ECS/EntityManager.h"
#include"Core/Utils/NonCopyable.h"

namespace PrCore::ECS {
	
	class EntityViewer: public  Utils::NonCopyable {
	public:
		EntityViewer(EntityManager* p_entityManager):
		m_entityManager(p_entityManager)
		{}

		~EntityViewer() = default;

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
