#pragma once
#include "Core/Utils/Assert.h"

namespace PrCore::ECS {

	template<class T>
	ComponentPool<T>::ComponentPool()
	{
		std::fill(m_components.begin(), m_components.end(), nullptr);
	}

	template<class T>
	ComponentPool<T>::~ComponentPool()
	{
		for (auto& component : m_components)
		{
			if (component != nullptr)
			{
				delete component;
				component = nullptr;
			}
		}
	}

	template<class T>
	T* ComponentPool<T>::AllocateData(ID p_ID)
	{
		static_assert(std::is_base_of<BaseComponent, T>::value, "Component must expand PrCore::ECS::BaseComponent");

		PR_ASSERT(p_ID.GetIndex() <= MAX_ENTITIES, "Wrong ID");
		PR_ASSERT(p_ID.IsValid(), "Wrong ID");

		auto entityIndex = p_ID.GetIndex() - 1;
		if (m_components[entityIndex])
		{
			PR_ASSERT(false, "Entity already has component " + std::string(typeid(T).name()));
			return nullptr;
		}

		m_components[entityIndex] = new T();
		return m_components[entityIndex];
	}

	template<class T>
	T* ComponentPool<T>::GetData(ID p_ID)
	{
		static_assert(std::is_base_of<BaseComponent, T>::value, "Component must expand PrCore::ECS::BaseComponent");

		PR_ASSERT(p_ID.GetIndex() <= MAX_ENTITIES, "Wrong ID");
		PR_ASSERT(p_ID.IsValid(), "Wrong ID");

		auto entityIndex = p_ID.GetIndex() - 1;
		PR_ASSERT(m_components[entityIndex], "Entity does not have component " + std::string(typeid(T).name()));
		return m_components[entityIndex];
	}

	template<class T>
	void ComponentPool<T>::RemoveData(ID p_ID)
	{
		static_assert(std::is_base_of<BaseComponent, T>::value, "Component must expand PrCore::ECS::BaseComponent");

		PR_ASSERT(p_ID.GetIndex() <= MAX_ENTITIES, "Wrong ID");
		PR_ASSERT(p_ID.IsValid(), "Wrong ID");

		auto entityIndex = p_ID.GetIndex() - 1;
		PR_ASSERT(m_components[entityIndex], "Entity does not have component " + std::string(typeid(T).name()));

		delete m_components[entityIndex];
		m_components[entityIndex] = nullptr;
	}

	template<class T>
	bool ComponentPool<T>::DataExist(ID p_ID)
	{
		static_assert(std::is_base_of<BaseComponent, T>::value, "Component must expand PrCore::ECS::BaseComponent");

		PR_ASSERT(p_ID.GetIndex() <= MAX_ENTITIES, "Wrong ID");
		PR_ASSERT(p_ID.IsValid(), "Wrong ID");

		auto entityIndex = p_ID.GetIndex() - 1;
		return m_components[entityIndex] != nullptr;
	}

	template<class T>
	void ComponentPool<T>::EntityDestroyed(ID p_ID)
	{
		static_assert(std::is_base_of<BaseComponent, T>::value, "Component must expand PrCore::ECS::BaseComponent");

		PR_ASSERT(p_ID.GetIndex() <= MAX_ENTITIES, "Wrong ID");
		PR_ASSERT(p_ID.IsValid(), "Wrong ID");

		RemoveData(p_ID);
	}

	template<class T>
	 BaseComponent* ComponentPool<T>::GetRawData(ID p_ID)
	 {
		 static_assert(std::is_base_of<BaseComponent, T>::value, "Component must expand PrCore::ECS::BaseComponent");

		 PR_ASSERT(p_ID.GetIndex() <= MAX_ENTITIES, "Wrong ID");
		 PR_ASSERT(p_ID.IsValid(), "Wrong ID");

		 T* component = GetData(p_ID);
		 return reinterpret_cast<BaseComponent*>(component);
	 }
}