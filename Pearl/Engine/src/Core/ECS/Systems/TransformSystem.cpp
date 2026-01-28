#include "Core/Common/pearl_pch.h"

#include "Core/ECS/Systems/TransformSystem.h"

#include "Core/ECS/Components.h"

using namespace PrCore;

void HierarchyTransform::OnCreate()
{
	m_updateGroup = static_cast<uint8_t>(UpdateGroupType::Custom);
}

void HierarchyTransform::OnUpdate(float p_dt)
{
	m_entityViewer.MT_HierarchicalEntitiesWithComponents<ParentComponent, TransformComponent>([](const Entity entity, ParentComponent* parentComponent, TransformComponent* transform)
		{
			auto parent = parentComponent->parent;
			if (!parent.IsValid())
				return;

			auto parentTransform = parent.GetComponent<TransformComponent>();

			transform->SetWorldMatrix(parentTransform->GetWorldMatrix() * transform->GetLocalMatrix());
			transform->DecomposeWorldMatrix();
		});
}