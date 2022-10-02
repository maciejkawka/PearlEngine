#pragma once
#include "Core/ECS/BaseSystem.h"
#include"Core/Events/Event.h"
#include"Core/ECS/EntityManager.h"

#include<tuple>
#include<unordered_set>

namespace PrCore::ECS {

	class HierarchyTransform: public BaseSystem {
	public:
		HierarchyTransform() = default;

		void OnCreate() override;
		void OnUpdate(float p_dt) override;

		void OnComponentAdded(Events::EventPtr p_event);
		void OnComponentRemoved(Events::EventPtr p_event);

	private:
		void UpdateHierarchyVector();
		int RecursiveHierarchyCreation(Entity p_parent, int p_depthIndex);

		std::vector<std::tuple<int, Entity>> m_hierarchyEntites;
		std::unordered_set<Entity> m_rootEntities;
		bool m_isDirty;
	};
}