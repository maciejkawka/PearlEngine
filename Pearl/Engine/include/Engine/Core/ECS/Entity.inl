//#pragma once
//
//namespace PrCore::ECS {
//
//	template<class T>
//	T* Entity::AddComponent()
//	{
//		PR_ASSERT(m_entityManager != nullptr, "EntityManager is nullptr");
//		return m_entityManager->AddComponent<T>(m_ID);
//	}
//
//	template<class T>
//	T* Entity::GetComponent()
//	{
//		PR_ASSERT(m_entityManager != nullptr, "EntityManager is nullptr");
//		return m_entityManager->GetComponent<T>(m_ID);
//	}
//
//	template<class T>
//	void Entity::RemoveComponent()
//	{
//		PR_ASSERT(m_entityManager != nullptr, "EntityManager is nullptr");
//		return m_entityManager->RemoveComponent<T>(m_ID);
//	}
//
//	template<class T>
//	bool Entity::HasComponent()
//	{
//		PR_ASSERT(m_entityManager != nullptr, "EntityManager is nullptr");
//		return m_entityManager->HasComponent<T>(m_ID);
//	}
//}