#include"Core/Common/pearl_pch.h"

#include"Core/ECS/Systems/TransformSystem.h"

#include "Core/ECS/Components.h"

using namespace PrCore::ECS;

void HierarchyTransform::OnCreate()
{
	m_updateGroup = static_cast<uint8_t>(UpdateGroup::Custom);
}

void HierarchyTransform::OnUpdate(float p_dt)
{
	for(auto [entity, transform]: m_entityViewer.HierarchicalEntitiesWithComponents<TransformComponent>())
	{
		auto parentComponent = entity.GetComponent<ParentComponent>();
		auto parent = parentComponent->parent;
		auto parentTransform = parent.GetComponent<TransformComponent>();

		transform->SetWorldMatrix(parentTransform->GetWorldMatrix() * transform->GetLocalMatrix());
		transform->DecomposeWorldMatrix();
	}
}