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
		Entity():
			m_ID(INVALID_ID),
			m_entityManager(nullptr)
		{}
		Entity(ID p_ID, EntityManager* p_entityManager) : m_ID(p_ID), m_entityManager(p_entityManager) {}
		Entity(const Entity& other) = default;
		Entity& operator = (const Entity& other) = default;

		inline ID GetID() const { return m_ID; }

		void Destroy(); //Not very safe should be removed
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
		//Local Classes
		class BasicIterator {
		public:
			using iterator_category = std::input_iterator_tag;
			using difference_type = Entity;

			BasicIterator() = delete;
			explicit BasicIterator(size_t p_index, EntityManager* p_entityManager, size_t p_entitiesNumber) :
				m_entityManager(p_entityManager),
				m_entitiesNumber(p_entitiesNumber),
				m_index(p_index)
			{}

			Entity operator*() const
			{
				PR_ASSERT(m_index < m_entitiesNumber, "Iterator out of range");
				return m_entityManager->ConstructEntityonIndex(m_index + 1);
			}
			bool operator==(const BasicIterator& p_other) const { return m_index == p_other.m_index; }
			bool operator!=(const BasicIterator& p_other) const { return m_index != p_other.m_index; }
			BasicIterator& operator++()
			{
				do {
					m_index++;
				} while (m_entityManager->m_entitiesSignature[m_index].none() && m_index < m_entitiesNumber);

				return *this;
			}

		protected:
			EntityManager* m_entityManager;
			size_t m_entitiesNumber;
			size_t m_index;
		};

		class TypedIterator : public BasicIterator {
		public:
			TypedIterator() = delete;
			explicit TypedIterator(size_t p_index, EntityManager* p_entityManager, size_t p_entitiesNumber, ComponentSignature p_mask) :
				BasicIterator(p_index, p_entityManager, p_entitiesNumber),
				m_mask(p_mask)
			{}

			BasicIterator& operator++()
			{
				do {
					m_index++;
				} while (m_index < m_entitiesNumber && (m_entityManager->m_entitiesSignature[m_index] & m_mask) != m_mask);

				return *this;
			}

		protected:
			ComponentSignature m_mask;
		};
		
		class BasicView {
		public:
			BasicView() = delete;
			explicit BasicView(EntityManager* p_entityManager) :
				m_entityManager(p_entityManager)
			{}

			BasicIterator begin() const
			{
				int index = 0;
				while (index < m_entityManager->m_entitiesNumber && m_entityManager->m_entitiesSignature[index].none())
					index++;

				return BasicIterator(index, m_entityManager, m_entityManager->m_entitiesNumber);
			}

			BasicIterator end() const
			{
				return BasicIterator(m_entityManager->m_entitiesNumber, m_entityManager, m_entityManager->m_entitiesNumber);
			}
		private:
			EntityManager* m_entityManager;
		};

		template<typename... ComponentTypes>
		class TypedView {
		public:
			TypedView() = delete;
			explicit TypedView(EntityManager* p_entityManager) :
				m_entityManager(p_entityManager)
			{
				if (sizeof...(ComponentTypes) == 0)
					PR_ASSERT(sizeof...(ComponentTypes) != 0, "No Component Specitied in ComponentWithComponents");
				else
				{
					size_t componentIDs[] = { m_entityManager->GetTypeID<ComponentTypes>() ... };
					for (int i = 0; i < (sizeof...(ComponentTypes)); i++)
						m_mask.set(componentIDs[i]);
				}
			}

			TypedIterator begin() const
			{
				int index = 0;
				while (index < m_entityManager->m_entitiesNumber && (m_entityManager->m_entitiesSignature[index] & m_mask) != m_mask)
					index++;

				return TypedIterator(index, m_entityManager, m_entityManager->m_entitiesNumber, m_mask);
			}

			TypedIterator end() const
			{
				return TypedIterator(m_entityManager->m_entitiesNumber, m_entityManager, m_entityManager->m_entitiesNumber, m_mask);
			}
		private:
			EntityManager* m_entityManager;
			ComponentSignature m_mask;
		};

		//Methods
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

		template<typename... ComponentTypes>
		TypedView<ComponentTypes...> GetEntitiesWithComponents();

		BasicView GetAllEntities();

		inline size_t GetEntityCount() const { return m_entitiesNumber; }

	private:
		void FireEntityCreated(Entity p_entity);
		void FireEntityDestoryed(Entity p_entity);

		template<class T>
		void FireComponentAdded(Entity p_entity, T* p_component);

		template<class T>
		void FireComponentRemoved(Entity p_entity, T* p_component);

		Entity ConstructEntityonIndex(uint32_t p_index);

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