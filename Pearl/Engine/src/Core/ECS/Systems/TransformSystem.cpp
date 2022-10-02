#include"Core/Common/pearl_pch.h"

#include"Core/ECS/Systems/TransformSystem.h"
#include"Core/ECS/Components/TransformComponent.h"
#include"Core/Events/EventManager.h"
#include "Core/Events/ECSEvents.h"

using namespace PrCore::ECS;

void HierarchyTransform::OnCreate()
{
	m_updateGroup = (uint8_t)UpdateGroup::Update;

	Events::EventListener componentAddedListener;
	componentAddedListener.connect<&HierarchyTransform::OnComponentAdded>(this);
	Events::EventManager::GetInstance().AddListener(componentAddedListener, Events::ComponentAddedEvent<ParentComponent>::s_type);

	Events::EventListener componentRemovedListener;
	componentRemovedListener.connect<&HierarchyTransform::OnComponentRemoved>(this);
	Events::EventManager::GetInstance().AddListener(componentRemovedListener, Events::ComponentRemovedEvent<ParentComponent>::s_type);
	m_isDirty = true;
}

void HierarchyTransform::OnUpdate(float p_dt)
{
	if (m_isDirty)
		UpdateHierarchyVector();

	for(auto entityPair : m_hierarchyEntites)
	{
		auto entity = std::get<1>(entityPair);
		auto transform = entity.GetComponent<TransformComponent>();

		auto parentComponent = entity.GetComponent<ParentComponent>();
		auto parent = parentComponent->parent;
		auto parentTransform = parent.GetComponent<TransformComponent>();

		transform->SetWorldMatrix(parentTransform->GetWorldMatrix() * transform->GetLocalMatrix());
		transform->DecomposeWorldMatrix();
	}
}

void HierarchyTransform::OnComponentAdded(Events::EventPtr p_event)
{
	auto componentAddedEvent = std::static_pointer_cast<Events::ComponentAddedEvent<ParentComponent>>(p_event);
	auto entity = componentAddedEvent->m_entity;

	m_hierarchyEntites.emplace_back(std::make_tuple(0, entity));
	m_isDirty = true;
}

void HierarchyTransform::OnComponentRemoved(Events::EventPtr p_event)
{
	auto componentAddedEvent = std::static_pointer_cast<Events::ComponentAddedEvent<ParentComponent>>(p_event);
	auto entity = componentAddedEvent->m_entity;

	auto foundEntity = std::find_if(m_hierarchyEntites.begin(), m_hierarchyEntites.end(), [&](std::tuple<int, Entity> p_element) -> bool
		{
			return std::get<1>(p_element).GetID() == entity.GetID();
		});

	if (foundEntity != m_hierarchyEntites.end())
	{
		m_hierarchyEntites.erase(foundEntity);
		m_isDirty = true;
	}
}

void HierarchyTransform::UpdateHierarchyVector()
{
	m_rootEntities.clear();

	//Get each element depth
	for(auto& element: m_hierarchyEntites)
	{
		auto& index = std::get<0>(element);
		auto entity = std::get<1>(element);

		index = RecursiveHierarchyCreation(entity, 0);
	}
	
	//Sort all entities form root to leafs
	std::sort(m_hierarchyEntites.begin(), m_hierarchyEntites.end(), [](const std::tuple<int, Entity> a, const std::tuple<int, Entity> b)->bool
		{
			return std::get<0>(a) < std::get<0>(b);
		});

	m_isDirty = false;
}

int HierarchyTransform::RecursiveHierarchyCreation(Entity p_entity, int p_depthIndex)
{
	if (!p_entity.HasComponent<ParentComponent>())
	{
		m_rootEntities.insert(p_entity);
		return p_depthIndex;
	}
	
	auto parent = p_entity.GetComponent<ParentComponent>();
	if (!parent->parent.IsValid())
		return p_depthIndex;

	return RecursiveHierarchyCreation(parent->parent, ++p_depthIndex);
}
