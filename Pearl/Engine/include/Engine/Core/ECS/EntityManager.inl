#pragma once
#include "Core/Events/ECSEvents.h"
#include "Core/Events/EventManager.h"

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
	bool Entity::HasComponent() const
	{
		PR_ASSERT(m_entityManager != nullptr, "EntityManager is nullptr");
		return m_entityManager->HasComponent<T>(m_ID);
	}

	template<class T>
	T* EntityManager::AddComponent(ID p_ID)
	{
		PR_ASSERT(IsValid(p_ID), std::string("ID is invalid"));

		if(m_ComponentPools.find(GetTypeID<T>()) == m_ComponentPools.end())
			RegisterComponent<T>();

		m_entitiesSignature[p_ID.GetIndex() - 1].set(GetTypeID<T>());
		auto componentPool = GetComponentPool<T>();
		auto component = componentPool->AllocateData(p_ID);
		FireComponentAdded<T>(ConstructEntityonIndex(p_ID.GetIndex()), component);
		return component;
	}

	template<class T>
	T* EntityManager::GetComponent(ID p_ID)
	{
		PR_ASSERT(IsValid(p_ID), std::string("ID is invalid"));

		auto componentPool = GetComponentPool<T>();
		return componentPool->GetData(p_ID);
	}

	template<class T>
	void EntityManager::RemoveComponent(ID p_ID)
	{
		PR_ASSERT(IsValid(p_ID), std::string("ID is invalid"));

		m_entitiesSignature[p_ID.GetIndex() - 1].reset(GetTypeID<T>());
		auto componentPool = GetComponentPool<T>();

		FireComponentRemoved<T>(ConstructEntityonIndex(p_ID.GetIndex()), componentPool->GetData(p_ID));
		componentPool->RemoveData(p_ID);
	}

	template<class T>
	bool EntityManager::HasComponent(ID p_ID)
	{
		PR_ASSERT(IsValid(p_ID), std::string("ID is invalid"));
		return m_entitiesSignature[p_ID.GetIndex() - 1].test(GetTypeID<T>());
	}

	template<class T>
	void EntityManager::RegisterComponent()
	{
		PR_ASSERT(s_typeComponentCounter < MAX_COMPONENTS, "Cannot register more components");

		auto componentID = GetTypeID<T>();
		m_ComponentPools[componentID] = std::make_shared<ComponentPool<T>>();;
		m_ComponentRemovers[componentID] = std::make_shared<ComponentRemover<T>>();;
	}

	template<typename ...ComponentTypes>
	EntityManager::TypedView<ComponentTypes...> EntityManager::GetEntitiesWithComponents()
	{
		return TypedView<ComponentTypes...>(this);
	}

	template<typename ...ComponentTypes>
	EntityManager::TypedHierarchicalView<ComponentTypes...> EntityManager::GetHierrarchicalEntitiesWithComponents()
	{
		return TypedHierarchicalView<ComponentTypes...>(this);
	}

	template<class T>
	void EntityManager::FireComponentAdded(Entity p_entity, T* p_component)
	{
		Events::EventPtr event = std::make_shared<Events::ComponentAddedEvent<T>>(p_entity, p_component);
		Events::EventManager::GetInstance().FireEvent(event);
	}

	template<class T>
	void EntityManager::FireComponentRemoved(Entity p_entity, T* p_component)
	{
		Events::EventPtr event = std::make_shared<Events::ComponentRemovedEvent<T>>(p_entity, p_component);
		Events::EventManager::GetInstance().FireEvent(event);
	}

	template<class T>
	size_t EntityManager::GetTypeID()
	{
		static size_t s_componentID = s_typeComponentCounter++;
		return s_componentID;
	}

	template<class T>
	std::shared_ptr<ComponentPool<T>> EntityManager::GetComponentPool()
	{
		auto componentID = GetTypeID<T>();
		auto findComponentPool = m_ComponentPools.find(componentID);
		PR_ASSERT(findComponentPool != m_ComponentPools.end(), "Component not registered " + std::string(typeid(T).name()));

		return std::static_pointer_cast<ComponentPool<T>>(findComponentPool->second);
	}
}