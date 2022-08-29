#pragma once
#include "Core/ECS/ComponentPool.h"

#include "Core/Utils/NonCopyable.h"

#include<vector>
#include<bitset>
#include<queue>
#include<memory>

namespace PrCore::ECS {

	class EntityManager;

	class Entity {
	public:
		Entity() = delete;
		Entity(ID p_ID, EntityManager* p_entityManager) : m_ID(p_ID), m_entityManager(p_entityManager) {}
		Entity(const Entity& other) = default;
		Entity& operator = (const Entity& other) = default;

		inline ID GetID() const { return m_ID; }

		void Destroy();
		bool IsValid() const;

		//Components
		template<class T>
		T* AddComponent();

		template<class T>
		T* GetComponent();

		template<class T>
		void RemoveComponent();

		template<class T>
		bool HasComponent();

	private:
		void Invalidate();

		ID m_ID;
		EntityManager* m_entityManager;
	};

	class EntityManager: public Utils::NonCopyable {
	public:
		EntityManager();

		Entity CreateEntity();
		void DestoryEntity(ID p_ID);

		bool IsValid(ID p_ID) const;

		//Components
		template<class T>
		T* AddComponent(ID p_ID);

		template<class T>
		T* GetComponent(ID p_ID);

		template<class T>
		void RemoveComponent(ID p_ID);

		template<class T>
		bool HasComponent(ID p_ID);

		template<class T>
		void RegisterComponent();

	private:
		template<class T>
		size_t GetTypeID();

		template<class T>
		std::shared_ptr<ComponentPool<T>> GetComponentPool();

		//Number of actual entites
		size_t m_entitiesNumber;

		//Vector holds signatures for all entites
		std::vector<ComponentSignature> m_entitiesSignature;

		//Vector holds version for all entities
		std::vector<uint32_t> m_entitiesVersion;

		//Queue with all free entity IDs
		std::queue<ID> m_freeEntitiesID;

		size_t m_typeComponentCounter = 0;

		//maps holds all components in pack array
		std::unordered_map<size_t, std::shared_ptr<IComponentPool>> m_ComponentPools;
	};
}

#include "Core/ECS/EntityManager.inl"