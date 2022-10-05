#include"Core/Common/pearl_pch.h"

#include"Core/ECS/Systems/TransformSystem.h"

#include "Core/ECS/Components.h"

using namespace PrCore::ECS;

void HierarchyTransform::OnCreate()
{
	m_updateGroup = static_cast<uint8_t>(UpdateGroup::Update);
}

void HierarchyTransform::OnUpdate(float p_dt)
{
	for(auto entity: m_entityViewer.HierarchicalEntitiesWithComponents<TransformComponent>())
	{
		auto transform = entity.GetComponent<TransformComponent>();

		auto parentComponent = entity.GetComponent<ParentComponent>();
		auto parent = parentComponent->parent;
		auto parentTransform = parent.GetComponent<TransformComponent>();

		transform->SetWorldMatrix(parentTransform->GetWorldMatrix() * transform->GetLocalMatrix());
		transform->DecomposeWorldMatrix();
	}
}