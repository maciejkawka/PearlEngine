#pragma once
#include"Core/ECS/EntityManager.h"
#include"Core/Threading/JobSystem.h"

namespace PrCore::ECS {

	constexpr size_t g_MTBatchSize = 256;

	template<typename T, typename Func>
	void JobBatchWork(T itBegin, T itEnd, size_t batchSize, Func funcPtr)
	{
		int i = 0;
		for (; itBegin != itEnd && i < batchSize; ++itBegin, ++i)
		{
			std::apply(funcPtr, *itBegin);
		}
	}

	template<typename T, typename Func>
	Threading::BatchJobState ScheduleBatchJobWork(T itBegin, T itEnd, size_t batchSize, Func funcPtr)
	{
		auto jobPtr = Threading::JobSystem::GetInstancePtr();
		Threading::BatchJobState batchState;
		while (itBegin != itEnd)
		{
			batchState += jobPtr->Schedule("ECS_Batch_Work", &JobBatchWork<T, Func>, itBegin, itEnd, batchSize, funcPtr);

			for (int i = 0; i < batchSize && itBegin != itEnd; i++)
				++itBegin;
		}

		return batchState;
	}

	class EntityViewer {
	public:
		EntityViewer() = delete;
		EntityViewer(EntityManager* p_entityManager) :
			m_entityManager(p_entityManager)
		{}

		// Single Thread Versions
		//--------------------------------------------------------
		template<typename... ComponentType>
		EntityManager::TypedView<ComponentType...> EntitesWithComponents()
		{
			return m_entityManager->GetEntitiesWithComponents<ComponentType...>();
		}

		EntityManager::BasicView AllEntities()
		{
			return m_entityManager->GetAllEntities();
		}

		EntityManager::BasicHierarchicalView AllHierarchicalEntities()
		{
			return m_entityManager->GetAllHierrarchicalEntities();
		}

		template<typename... ComponentType>
		EntityManager::TypedHierarchicalView<ComponentType...> HierarchicalEntitiesWithComponents()
		{
			return m_entityManager->GetHierrarchicalEntitiesWithComponents<ComponentType...>();
		}


		// Multi Thread Versions
		//--------------------------------------------------------
		template<size_t BatchSize = g_MTBatchSize>
		void MT_AllEntities(std::function<void(ECS::Entity)> jobFunction)
		{
			static_assert(BatchSize > 0, "BatchSize cannot be 0");
			PR_ASSERT(jobFunction, "Function is null");

			auto entityView = m_entityManager->GetAllEntities();
			auto batchJobState = ScheduleBatchJobWork(entityView.begin(), entityView.end(), BatchSize, jobFunction);
			batchJobState.Wait();
		}

		template<typename... ComponentType>
		void MT_EntitesWithComponents(std::function<void(ECS::Entity, std::add_pointer_t<ComponentType>...)> jobFunction)
		{
			MT_EntitesWithComponents<g_MTBatchSize, ComponentType...>(jobFunction);
		}

		template<size_t BatchSize, typename... ComponentType>
		void MT_EntitesWithComponents(std::function<void(ECS::Entity, std::add_pointer_t<ComponentType>...)> jobFunction)
		{
			static_assert(BatchSize > 0, "BatchSize cannot be 0");
			PR_ASSERT(jobFunction, "Function is null");

			EntityManager::TypedView<ComponentType...> entityView = m_entityManager->GetEntitiesWithComponents<ComponentType...>();
			auto batchJobState = ScheduleBatchJobWork(entityView.begin(), entityView.end(), BatchSize, jobFunction);
			batchJobState.Wait();
		}

		template<size_t BatchSize = g_MTBatchSize>
		void  MT_AllHierarchicalEntities(std::function<void(ECS::Entity)> jobFunction)
		{
			static_assert(BatchSize > 0, "BatchSize cannot be 0");
			PR_ASSERT(jobFunction, "Function is null");

			auto entityView = m_entityManager->GetAllHierrarchicalEntities();
			auto viewIt = entityView.begin();

			// Run only same generation children in parallel
			Threading::BatchJobState batchJobState;
			auto vecIt = m_entityManager->m_hierarchicalEntites.begin();
			auto vecItEnd = m_entityManager->m_hierarchicalEntites.end();
			while (vecIt != vecItEnd)
			{
				// gather same generation entities and execute them in the batch job
				// wait for the batch job after next generation is gathered saves a time spent on main thread
				size_t generationDepth = vecIt->first;
				auto viewBegin = viewIt;
				while (vecIt != vecItEnd && vecIt->first == generationDepth)
				{
					++viewIt;
					++vecIt;
				}

				// first wait is on empty batch next one wait for the previous batch
				batchJobState.Wait();
				batchJobState = ScheduleBatchJobWork(viewBegin, viewIt, BatchSize, jobFunction);
				generationDepth++;
			}

			// final wait for the last batch that was created
			batchJobState.Wait();
		}

		template<typename... ComponentType>
		void MT_HierarchicalEntitiesWithComponents(std::function<void(ECS::Entity, std::add_pointer_t<ComponentType>...)> jobFunction)
		{
			MT_HierarchicalEntitiesWithComponents<g_MTBatchSize, ComponentType...>(jobFunction);
		}

		template<size_t BatchSize, typename... ComponentType>
		void MT_HierarchicalEntitiesWithComponents(std::function<void(ECS::Entity, std::add_pointer_t<ComponentType>...)> jobFunction)
		{
			static_assert(BatchSize > 0, "BatchSize cannot be 0");
			PR_ASSERT(jobFunction, "Function is null");

			auto entityView = m_entityManager->GetHierrarchicalEntitiesWithComponents<ComponentType...>();
			auto viewIt = entityView.begin();

			// Run only same generation children in parallel
			Threading::BatchJobState batchJobState;
			auto vecIt = m_entityManager->m_hierarchicalEntites.begin();
			auto vecItEnd = m_entityManager->m_hierarchicalEntites.end();
			while (vecIt != vecItEnd)
			{
				// gather same generation entities and execute them in the batch job
				// wait for the batch job after next generation is gathered saves a time spent on main thread
				size_t generationDepth = vecIt->first;
				auto viewBegin = viewIt;
				while (vecIt != vecItEnd && vecIt->first == generationDepth)
				{
					++viewIt;
					++vecIt;
				}

				// first wait is on empty batch next one wait for the previous batch
				batchJobState.Wait();
				batchJobState = ScheduleBatchJobWork(viewBegin, viewIt, BatchSize, jobFunction);
				generationDepth++;
			}

			// final wait for the last batch that was created
			batchJobState.Wait();
		}
	private:
		EntityManager* m_entityManager;
	};
}
