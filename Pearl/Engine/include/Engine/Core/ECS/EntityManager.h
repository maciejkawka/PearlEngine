#pragma once
#include "Core/ECS/ComponentPool.h"

#include "Core/Utils/NonCopyable.h"
#include "Core/Utils/ISerializable.h"

#include"Core/Events/Event.h"

#include<vector>
#include<bitset>
#include<queue>
#include<memory>

namespace PrCore::ECS {

	class EntityManager;
	class EntityViewer;

	class Entity {
	public:
		Entity():
			m_ID(INVALID_ID),
			m_entityManager(nullptr)
		{}
		Entity(ID p_ID, EntityManager* p_entityManager) : m_ID(p_ID), m_entityManager(p_entityManager) {}
		Entity(const Entity& other) = default;
		Entity(Entity&& other) = default;
		Entity& operator = (const Entity& other) = default;
		Entity& operator = (Entity&& other) = default;

		inline ID GetID() const { return m_ID; }

		void Destroy(); 
		bool IsValid() const;

		template<class T>
		T* AddComponent();

		template<class T>
		T* GetComponent();

		template<class T>
		void RemoveComponent();

		template<class T>
		bool HasComponent() const;

		ComponentSignature GetComponentSignature() const;

		inline bool operator<(const Entity& p_entity) const { return m_ID < p_entity.GetID(); }
		inline bool operator==(const Entity& p_entity) const { return m_ID == p_entity.GetID(); }
		inline bool operator!=(const Entity& p_entity) const { return m_ID != p_entity.GetID(); }

	private:
		void Invalidate();

		ID m_ID;
		EntityManager* m_entityManager;
	};

	class IComponentRemover {
	public:
		virtual ~IComponentRemover() = default;
		virtual void RemoveComponent(Entity p_entity) = 0;
	};

	template<class Component>
	class ComponentRemover : public IComponentRemover {
	public:
		inline void RemoveComponent(Entity p_entity) override
		{
			p_entity.RemoveComponent<Component>();
		}
	};

	template<typename ComponentType, typename... ComponentTypes>
	auto CreateComponentTupleHelper(Entity& p_entity)
	{
		if constexpr (sizeof...(ComponentTypes) == 0)
			return std::make_tuple(p_entity.GetComponent<ComponentType>());
		else
			return std::tuple_cat(std::make_tuple(p_entity.GetComponent<ComponentType>()), CreateComponentTupleHelper<ComponentTypes...>(p_entity));
	}

	template<typename... ComponentTypes>
	auto CreateComponentTuple(Entity& p_entity)
	{
		static_assert(sizeof...(ComponentTypes) != 0, "Cannot create a tuple, ComponentType is empty");
		return CreateComponentTupleHelper<ComponentTypes...>(p_entity);
	}

	class EntityManager: public Utils::NonCopyable, Utils::ISerializable {
	public:
		using HierarchicalPair = std::pair<int, Entity>;

		//Local Classes
		template<typename... ComponentTypes>
		class BasicIterator {
		public:
			using iterator_category = std::input_iterator_tag;
			using difference_type = std::tuple<Entity>;

			BasicIterator() = delete;
			explicit BasicIterator(size_t p_index, EntityManager* p_entityManager, size_t p_entitiesNumber) :
				m_entityManager(p_entityManager),
				m_entitiesNumber(p_entitiesNumber),
				m_index(p_index)
			{}
			virtual ~BasicIterator() = default;

			std::tuple<Entity> operator*() const
			{
				PR_ASSERT(m_index < m_entitiesNumber, "Iterator out of range");
				return std::tuple_cat(std::make_tuple(m_entityManager->ConstructEntityonIndex(m_index + 1)));
			}

			bool operator==(const BasicIterator<ComponentTypes...>& p_other) const { return m_index == p_other.m_index; }
			bool operator!=(const BasicIterator<ComponentTypes...>& p_other) const { return m_index != p_other.m_index; }
			virtual BasicIterator<ComponentTypes...>& operator++()
			{
				do {
					++m_index;
				} while (m_index < m_entitiesNumber && m_entityManager->m_entitiesSignature[m_index].none());

				return *this;
			}

		protected:
			EntityManager* m_entityManager;
			size_t m_entitiesNumber;
			size_t m_index;
		};

		template<typename... ComponentTypes>
		class TypedIterator : public BasicIterator<ComponentTypes...> {
		public:
			using difference_type = std::tuple<Entity, std::add_pointer_t<ComponentTypes>...>;
			using BasicIterator<ComponentTypes...>::m_entityManager;
			using BasicIterator<ComponentTypes...>::m_entitiesNumber; 
			using BasicIterator<ComponentTypes...>::m_index;

			TypedIterator() = delete;
			explicit TypedIterator(size_t p_index, EntityManager* p_entityManager, size_t p_entitiesNumber, ComponentSignature p_mask) :
				BasicIterator<ComponentTypes...>(p_index, p_entityManager, p_entitiesNumber),
				m_mask(p_mask)
			{}

			std::tuple<Entity, std::add_pointer_t<ComponentTypes>...> operator*() const
			{
				PR_ASSERT(m_index < m_entitiesNumber, "Iterator out of range");

				Entity entity = m_entityManager->ConstructEntityonIndex(m_index + 1);
				return  std::tuple_cat(std::make_tuple(entity), CreateComponentTuple<ComponentTypes...>(entity));
			}

			virtual BasicIterator<ComponentTypes...>& operator++() override
			{
				do {
					++m_index;
				} while (m_index < m_entitiesNumber && (m_entityManager->m_entitiesSignature[m_index] & m_mask) != m_mask);

				return *this;
			}

		protected:
			ComponentSignature m_mask;
		};

		template<typename... ComponentTypes>
		class HierarchicalIterator {
		public:
			using iterator_category = std::input_iterator_tag;
			using difference_type = std::tuple<Entity>;

			HierarchicalIterator() = delete;
			explicit HierarchicalIterator(size_t p_index, EntityManager* p_entityManager, size_t p_entitiesNumber) :
				m_entityManager(p_entityManager),
				m_entitiesNumber(p_entitiesNumber),
				m_index(p_index)
			{}
			virtual ~HierarchicalIterator() = default;

			std::tuple<Entity> operator*() const
			{
				PR_ASSERT(m_index < m_entitiesNumber, "Iterator out of range");
				return std::make_tuple(m_entityManager->m_hierarchicalEntites[m_index].second);
			}

			bool operator==(const HierarchicalIterator<ComponentTypes...>& p_other) const { return m_index == p_other.m_index; }
			bool operator!=(const HierarchicalIterator<ComponentTypes...>& p_other) const { return m_index != p_other.m_index; }
			virtual HierarchicalIterator<ComponentTypes...>& operator++()
			{
				++m_index;
				return *this;
			}
		protected:
			EntityManager* m_entityManager;
			size_t m_entitiesNumber;
			size_t m_index;
		};

		template<typename... ComponentTypes>
		class HierarchicalTypedIterator: public HierarchicalIterator<ComponentTypes...> {
		public:
			using difference_type = std::tuple<Entity, std::add_pointer_t<ComponentTypes>...>;
			using HierarchicalIterator<ComponentTypes...>::m_entityManager;
			using HierarchicalIterator<ComponentTypes...>::m_entitiesNumber;
			using HierarchicalIterator<ComponentTypes...>::m_index;

			HierarchicalTypedIterator() = delete;
			explicit HierarchicalTypedIterator(size_t p_index, EntityManager* p_entityManager, size_t p_entitiesNumber, ComponentSignature p_mask) :
				HierarchicalIterator<ComponentTypes...>(p_index, p_entityManager, p_entitiesNumber),
				m_mask(p_mask)
			{}

			std::tuple<Entity, std::add_pointer_t<ComponentTypes>...> operator*() const
			{
				PR_ASSERT(m_index < m_entitiesNumber, "Iterator out of range");

				Entity entity = m_entityManager->m_hierarchicalEntites[m_index].second;
				return  std::tuple_cat(std::make_tuple(entity), CreateComponentTuple<ComponentTypes...>(m_entityManager->m_hierarchicalEntites[m_index].second));
			}

			virtual HierarchicalTypedIterator<ComponentTypes...>& operator++() override
			{
				Entity nextEntity;
				do {
					++m_index;
				} while (m_index < m_entitiesNumber && (m_entityManager->m_hierarchicalEntites[m_index].second.GetComponentSignature() & m_mask) != m_mask);

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

			BasicIterator<void> begin() const
			{
				int index = 0;
				while (index < m_entityManager->m_entitiesSignature.size() && m_entityManager->m_entitiesSignature[index].none())
					++index;

				return BasicIterator<void>(index, m_entityManager, m_entityManager->m_entitiesSignature.size());
			}

			BasicIterator<void> end() const
			{
				return BasicIterator<void>(m_entityManager->m_entitiesSignature.size(), m_entityManager, m_entityManager->m_entitiesSignature.size());
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
				static_assert(sizeof...(ComponentTypes) != 0, "No Component Specitied in ComponentWithComponents");

				size_t componentIDs[] = { m_entityManager->GetTypeID<ComponentTypes>() ... };
				for (int i = 0; i < (sizeof...(ComponentTypes)); i++)
					m_mask.set(componentIDs[i]);
			}

			TypedIterator<ComponentTypes...> begin() const
			{
				int index = 0;
				while (index < m_entityManager->m_entitiesSignature.size() && (m_entityManager->m_entitiesSignature[index] & m_mask) != m_mask)
					++index;

				return TypedIterator<ComponentTypes...>(index, m_entityManager, m_entityManager->m_entitiesSignature.size(), m_mask);
			}

			TypedIterator<ComponentTypes...> end() const
			{
				return TypedIterator<ComponentTypes...>(m_entityManager->m_entitiesSignature.size(), m_entityManager, m_entityManager->m_entitiesSignature.size(), m_mask);
			}
		private:
			EntityManager* m_entityManager;
			ComponentSignature m_mask;
		};

		class BasicHierarchicalView {
		public:
			BasicHierarchicalView() = delete;
			explicit BasicHierarchicalView(EntityManager* p_entityManager);
			virtual ~BasicHierarchicalView() = default;

			HierarchicalIterator<void> begin() const
			{
				return HierarchicalIterator<void>(0, m_entityManager, m_entityManager->m_hierarchicalEntites.size());
			}

			HierarchicalIterator<void> end() const
			{
				return HierarchicalIterator<void>(m_entityManager->m_hierarchicalEntites.size(), m_entityManager, m_entityManager->m_hierarchicalEntites.size());
			}

		protected:
			EntityManager* m_entityManager;

			void UpdateHierarchicalEntites();
			int RecursiveHierarchyCreation(Entity p_entity, int p_depthIndex);
		};

		template<typename... ComponentTypes>
		class TypedHierarchicalView: public  BasicHierarchicalView {
		public:
			TypedHierarchicalView() = delete;
			explicit TypedHierarchicalView(EntityManager* p_entityManager) :
				BasicHierarchicalView(p_entityManager)
			{
				static_assert(sizeof...(ComponentTypes) != 0, "No Component Specitied in ComponentWithComponents");

				size_t componentIDs[] = { m_entityManager->GetTypeID<ComponentTypes>() ... };
				for (int i = 0; i < (sizeof...(ComponentTypes)); i++)
					m_mask.set(componentIDs[i]);
			}

			HierarchicalTypedIterator<ComponentTypes...> begin() const
			{
				int index = 0;
				auto hierarchicalEntities = m_entityManager->m_hierarchicalEntites;
				if (hierarchicalEntities.empty())
					return end();

				while (index < m_entityManager->m_hierarchicalEntites.size() && (hierarchicalEntities[index].second.GetComponentSignature() & m_mask) != m_mask)
					index++;

				return HierarchicalTypedIterator<ComponentTypes...>(index, m_entityManager, m_entityManager->m_hierarchicalEntites.size(), m_mask);
			}

			virtual HierarchicalTypedIterator<ComponentTypes...> end() const
			{
				auto size = m_entityManager->m_hierarchicalEntites.size();
				return HierarchicalTypedIterator<ComponentTypes...>(size, m_entityManager, size, m_mask);
			}

		protected:
			ComponentSignature m_mask;
		};

		//Methods
		EntityManager();
		~EntityManager();

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

		template<typename... ComponentTypes>
		TypedHierarchicalView<ComponentTypes...> GetHierrarchicalEntitiesWithComponents();

		BasicHierarchicalView GetAllHierrarchicalEntities();

		ComponentSignature GetComponentSignature(ID p_ID);

		inline size_t GetEntityCount() const { return m_entitiesNumber; }

		void OnSerialize(Utils::JSON::json& p_serialized) override;
		void OnDeserialize(const Utils::JSON::json& p_serialized) override;

	private:
		void FireEntityCreated(Entity p_entity);
		void FireEntityDestoryed(Entity p_entity);

		template<class T>
		void FireComponentAdded(Entity p_entity, T* p_component);

		template<class T>
		void FireComponentRemoved(Entity p_entity, T* p_component);

		Entity ConstructEntityonIndex(uint32_t p_index);

		//For Hierarchical Vector
		void OnParentComponentModified(Events::EventPtr p_event);

		template<class T>
		size_t GetTypeID();

		template<class T>
		std::shared_ptr<ComponentPool<T>> GetComponentPool();

		//Number of actual entities
		size_t m_entitiesNumber;

		//Vector holds signatures for all entities
		std::vector<ComponentSignature> m_entitiesSignature;

		//Vector holds version for all entities
		std::vector<uint32_t> m_entitiesVersion;

		//Queue with all free entity IDs
		std::queue<ID> m_freeEntitiesID;

		//maps holds all components in pack array
		std::unordered_map<size_t, std::shared_ptr<IComponentPool>> m_ComponentPools;

		//map holds all component in case to remove
		std::unordered_map<size_t, std::shared_ptr<IComponentRemover>> m_ComponentRemovers;

		//vector with hierarchical entities
		std::vector<HierarchicalPair> m_hierarchicalEntites;
		bool m_isHierarchicalEntitiesDirty = false;

		inline static size_t s_typeComponentCounter = 0;

		friend EntityViewer;
	};
}

namespace std {
	template<>
	struct hash<PrCore::ECS::Entity> {
		size_t operator()(const PrCore::ECS::Entity& p_entity) const {

			return p_entity.GetID().GetID();
		}
	};
}

#include "Core/ECS/EntityManager.inl"