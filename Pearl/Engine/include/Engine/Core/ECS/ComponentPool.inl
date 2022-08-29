#pragma once
#include "Core/Utils/Assert.h"

namespace PrCore::ECS {

	template<class T>
	ComponentPool<T>::ComponentPool():
		m_componentsNumber(0)
	{}

	template<class T>
	T* ComponentPool<T>::AllocateData(ID p_ID)
	{
		PR_ASSERT(m_entityToIndexMap.find(p_ID) == m_entityToIndexMap.end(), "Entity already has component " + std::string(typeid(T).name()));
		PR_ASSERT(p_ID.GetIndex() <= MAX_ENTITIES, "Wrong ID");

		auto nextComponentIndex = m_componentsNumber;
		m_entityToIndexMap[p_ID] = nextComponentIndex;
		m_indexToEntityMap[nextComponentIndex] = p_ID;
		m_components[nextComponentIndex] = T();
		m_componentsNumber++;

		return &m_components[nextComponentIndex];
	}

	template<class T>
	T* ComponentPool<T>::GetData(ID p_ID)
	{
		PR_ASSERT(m_entityToIndexMap.find(p_ID) != m_entityToIndexMap.end(), "Entity does not have component " + std::string(typeid(T).name()));
		PR_ASSERT(p_ID.GetIndex() <= MAX_ENTITIES, "Wrong ID");

		return &m_components[p_ID.GetIndex() - 1];
	}

	template<class T>
	void ComponentPool<T>::RemoveData(ID p_ID)
	{
		PR_ASSERT(m_entityToIndexMap.find(p_ID) != m_entityToIndexMap.end(), "Entity does not have component " + std::string(typeid(T).name()));
		PR_ASSERT(p_ID.GetIndex() <= MAX_ENTITIES, "Wrong ID");

		auto removedEntityIndex = m_entityToIndexMap[p_ID];
		auto lastComponentIndex = m_componentsNumber - 1;

		m_components[removedEntityIndex] = m_components[lastComponentIndex];

		auto lastEntity = m_indexToEntityMap[lastComponentIndex];
		m_entityToIndexMap[lastEntity] = removedEntityIndex;
		m_indexToEntityMap[removedEntityIndex] = lastEntity;

		m_entityToIndexMap.erase(p_ID);
		m_entityToIndexMap.erase(lastEntity);

		m_componentsNumber--;
	}

	template<class T>
	bool ComponentPool<T>::DataExist(ID p_ID)
	{
		return m_entityToIndexMap.find(p_ID) != m_entityToIndexMap.end();
	}
}