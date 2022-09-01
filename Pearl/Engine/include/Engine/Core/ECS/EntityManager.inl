#include "EntityManager.h"
#pragma once

namespace PrCore::ECS {

	template<class T>
	T* Entity::AddComponent()
	{
		PR_ASSERT(m_entityManager != nullptr, "EntityManager is nullptr");
		return m_entityManager->AddComponent<T>(m_ID);
	}

	template<class T>
	T* Entity::GetComponent()
	{
		PR_ASSERT(m_entityManager != nullptr, "EntityManager is nullptr");
		return m_entityManager->GetComponent<T>(m_ID);
	}

	template<class T>
	void Entity::RemoveComponent()
	{
		PR_ASSERT(m_entityManager != nullptr, "EntityManager is nullptr");
		return m_entityManager->RemoveComponent<T>(m_ID);
	}

	template<class T>
	bool Entity::HasComponent()
	{
		PR_ASSERT(m_entityManager != nullptr, "EntityManager is nullptr");
		return m_entityManager->HasComponent<T>(m_ID);
	}

	template<class T>
	T* EntityManager::AddComponent(ID p_ID)
	{
		PR_ASSERT(IsValid(p_ID), std::string("ID " + std::to_string(p_ID.GetID()) + "is invalid"));

		m_entitiesSignature[p_ID.GetIndex() - 1].set(GetTypeID<T>());
		auto componentPool = GetComponentPool<T>();
		return componentPool->AllocateData(p_ID);
	}

	template<class T>
	T* EntityManager::GetComponent(ID p_ID)
	{
		PR_ASSERT(IsValid(p_ID), std::string("ID " + std::to_string(p_ID.GetID()) + "is invalid"));

		auto componentPool = GetComponentPool<T>();
		return componentPool->GetData(p_ID);
	}

	template<class T>
	void EntityManager::RemoveComponent(ID p_ID)
	{
		PR_ASSERT(IsValid(p_ID), std::string("ID " + std::to_string(p_ID.GetID()) + "is invalid"));

		m_entitiesSignature[p_ID.GetIndex() - 1].reset(GetTypeID<T>());
		auto componentPool = GetComponentPool<T>();
		return componentPool->RemoveData(p_ID);
	}

	template<class T>
	bool EntityManager::HasComponent(ID p_ID)
	{
		PR_ASSERT(IsValid(p_ID), std::string("ID " + std::to_string(p_ID.GetID()) + "is invalid"));
		return m_entitiesSignature[p_ID.GetIndex() - 1].test(GetTypeID<T>());
	}

	template<class T>
	void EntityManager::RegisterComponent()
	{
		PR_ASSERT(m_typeComponentCounter < MAX_COMPONENTS, "Cannot register more components");

		auto componentID = GetTypeID<T>();
		auto componentPool = std::make_shared<ComponentPool<T>>();
		m_ComponentPools[componentID] = componentPool;
	}

	template<typename ...ComponentTypes>
	EntityManager::TypedView<ComponentTypes...> EntityManager::GetEntitiesWithComponents()
	{
		return TypedView<ComponentTypes...>(this);
	}

	template<class T>
	size_t EntityManager::GetTypeID()
	{
		static size_t s_componentID = m_typeComponentCounter++;
		return s_componentID;
	}

	template<class T>
	std::shared_ptr<ComponentPool<T>> EntityManager::GetComponentPool()
	{
		auto componentID = GetTypeID<T>();
		auto findComponentPool = m_ComponentPools.find(componentID);
		PR_ASSERT(findComponentPool != m_ComponentPools.end(), "Component not registered " + std::string(typeid(T).name()));

		return std::static_pointer_cast<ComponentPool<T>>(m_ComponentPools[componentID]);
	}
}