#include"Core/Common/pearl_pch.h"

#include "Core/ECS/EntityManager.h"
#include "Core/ECS/BaseComponent.h"
#include"Core/Events/EventManager.h"
#include"Core/Events/ECSEvents.h"
#include "Core/ECS/ComponentMap.h"

using namespace PrCore::ECS;

void Entity::Destroy()
{
	PR_ASSERT(m_entityManager != nullptr, "EntityManager is nullptr");

	AddComponent<ToDestoryTag>();
}

bool Entity::IsValid() const
{
	return m_entityManager && m_entityManager->IsValid(m_ID);
}

ComponentSignature Entity::GetComponentSignature() const
{
	return m_entityManager->GetComponentSignature(m_ID);
}

void Entity::Invalidate()
{
	m_ID = INVALID_ID;
}

EntityManager::BasicHierarchicalView::BasicHierarchicalView(EntityManager* p_entityManager) :
	m_entityManager(p_entityManager)
{
	if (m_entityManager->m_isHierarchicalEntitiesDirty)
		UpdateHierarchicalEntites();

	// Check if should recalculate index
	auto& hierarchicalEntities = m_entityManager->m_hierarchicalEntites;
	bool shouldRecalculate = false;
	for (auto& [_, entity] : hierarchicalEntities)
	{
		shouldRecalculate |= entity.GetComponent<ParentComponent>()->isDirty;
		entity.GetComponent<ParentComponent>()->isDirty = false;
	}

	// Calculate depth index
	if (shouldRecalculate)
	{
		for (auto& element : hierarchicalEntities)
		{
			auto& index = element.first;
			auto entity = element.second;

			index = RecursiveHierarchyCreation(entity, 0);
		}

		//Sort all entities form root to leafs
		std::sort(hierarchicalEntities.begin(), hierarchicalEntities.end(), [](const std::pair<int, Entity> a, const std::pair<int, Entity> b)->bool
			{
				return a.first < b.first;
			});
	}
}

void EntityManager::BasicHierarchicalView::UpdateHierarchicalEntites()
{
	auto& hierarchicalEntities = m_entityManager->m_hierarchicalEntites;
	hierarchicalEntities.clear();
	hierarchicalEntities.reserve(m_entityManager->GetEntityCount());

	// Create the vector
	EntityViewer viewer(m_entityManager);
	for (auto [entity, _] : viewer.EntitesWithComponents<ParentComponent>())
	{
		hierarchicalEntities.push_back(std::make_pair(0, entity));
	}

	m_entityManager->m_isHierarchicalEntitiesDirty = false;
}

int EntityManager::BasicHierarchicalView::RecursiveHierarchyCreation(Entity p_entity, int p_depthIndex)
{
	if (!p_entity.HasComponent<ParentComponent>())
		return p_depthIndex;

	auto parent = p_entity.GetComponent<ParentComponent>();
	if (!parent->parent.IsValid())
		return p_depthIndex;

	return RecursiveHierarchyCreation(parent->parent, ++p_depthIndex);
}

EntityManager::EntityManager():
	m_entitiesNumber(0)
{
	Events::EventListener parentComponentModified;
	parentComponentModified.connect<&EntityManager::OnParentComponentModified>(this);
	Events::EventManager::GetInstance().AddListener(parentComponentModified, Events::ComponentAddedEvent<ParentComponent>::s_type);
	Events::EventManager::GetInstance().AddListener(parentComponentModified, Events::ComponentRemovedEvent<ParentComponent>::s_type);
}

EntityManager::~EntityManager()
{
	Events::EventListener parentComponentModified;
	parentComponentModified.connect<&EntityManager::OnParentComponentModified>(this);
	Events::EventManager::GetInstance().RemoveListener(parentComponentModified, Events::ComponentAddedEvent<ParentComponent>::s_type);
	Events::EventManager::GetInstance().RemoveListener(parentComponentModified, Events::ComponentRemovedEvent<ParentComponent>::s_type);
}

Entity EntityManager::CreateEntity()
{
	ID newID;

	if(!m_freeEntitiesID.empty())
	{
		ID reusedID = m_freeEntitiesID.front();
		m_freeEntitiesID.pop();

		newID = ID(reusedID.GetIndex(), reusedID.GetVersion() + 1);
	}
	else
	{
		newID = ID(m_entitiesSignature.size() + 1, 1);
		m_entitiesSignature.push_back(ComponentSignature());
		m_entitiesVersion.push_back(1);
	}

	m_entitiesNumber++;

	return Entity(newID, this);
}

void EntityManager::DestoryEntity(ID p_ID)
{
	PR_ASSERT(IsValid(p_ID), std::string("ID " + std::to_string(p_ID.GetID()) + "is invalid"));

	//Remove all components
	auto entitySignature = m_entitiesSignature[p_ID.GetIndex() - 1];
	for (const auto& componentRemoverPair : m_ComponentRemovers)
	{
		//Small  if statement to faster find if component exist
		if (entitySignature.test(componentRemoverPair.first))
			componentRemoverPair.second->RemoveComponent(ConstructEntityonIndex(p_ID.GetIndex()));
	}

	auto index = p_ID.GetIndex();
	m_entitiesSignature[index - 1].reset();
	m_entitiesVersion[index - 1]++;
	m_entitiesNumber--;
	m_freeEntitiesID.push(p_ID);
}

bool EntityManager::IsValid(ID p_ID) const
{
	return p_ID != INVALID_ID &&
		p_ID.GetIndex() <= m_entitiesSignature.size() &&
		p_ID.GetVersion() == m_entitiesVersion[p_ID.GetIndex() - 1];
}

EntityManager::BasicView EntityManager::GetAllEntities()
{
	return BasicView(this);
}

EntityManager::BasicHierarchicalView EntityManager::GetAllHierrarchicalEntities()
{
	return BasicHierarchicalView(this);
}

ComponentSignature EntityManager::GetComponentSignature(ID p_ID)
{
	PR_ASSERT(IsValid(p_ID), std::string("ID " + std::to_string(p_ID.GetID()) + "is invalid"));
	return m_entitiesSignature[p_ID.GetIndex() - 1];
}

void EntityManager::OnSerialize(Utils::JSON::json& p_serialized)
{
	//Serialize Entities
	for (int i=1;i<=m_entitiesNumber;i++)
	{
		Utils::JSON::json entityJSON;
		auto entity = ConstructEntityonIndex(i);
		auto ID = entity.GetID();
		auto entitySignature = m_entitiesSignature[i - 1];

		entityJSON["ID"] = ID.GetID();

		//Serialize Components on Entity
		Utils::JSON::json componentsJSON;
		for (int j = 0; j < entitySignature.size(); j++)
		{
			if (entitySignature.test(j))
			{
				auto componentPool = m_ComponentPools[j];
				auto component = componentPool->GetRawData(ID);

				Utils::JSON::json serializedComponents;
				serializedComponents["componentType"] = typeid(*component).name();
				component->OnSerialize(serializedComponents);

				componentsJSON.push_back(serializedComponents);
			}
		}

		entityJSON["components"] = componentsJSON;

		p_serialized.push_back(entityJSON);
	}
}

void EntityManager::OnDeserialize(const Utils::JSON::json& p_serialized)
{
	for(auto& entityJSON : p_serialized)
	{
		auto entity = CreateEntity();

		auto components = entityJSON["components"];
		for(auto& componentJSON: components)
		{
			std::string componentType = componentJSON["componentType"];
			auto component = DeduceComponentTypeByString(entity, componentType);
			component->OnDeserialize(componentJSON);
		}
	}
}

void EntityManager::FireEntityCreated(Entity p_entity)
{
	Events::EventPtr event = std::make_shared<Events::EntityCreatedEvent>(p_entity);
	Events::EventManager::GetInstance().FireEvent(event);
}

void EntityManager::FireEntityDestoryed(Entity p_entity)
{
	Events::EventPtr event = std::make_shared<Events::EntityDestroyedEvent>(p_entity);
	Events::EventManager::GetInstance().FireEvent(event);
}

Entity EntityManager::ConstructEntityonIndex(uint32_t p_index)
{
	auto entityID = ID(p_index, m_entitiesVersion[p_index - 1]);
	return Entity(entityID, this);
}

void EntityManager::OnParentComponentModified(Events::EventPtr p_event)
{
	m_isHierarchicalEntitiesDirty = true;
}
