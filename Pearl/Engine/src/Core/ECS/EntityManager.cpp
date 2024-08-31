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
	Invalidate();
}

bool Entity::IsValid() const
{
	PR_ASSERT(m_entityManager != nullptr, "EntityManager is nullptr");

	return m_entityManager->IsValid(m_ID);
}

ComponentSignature Entity::GetComponentSignature() const
{
	return m_entityManager->GetComponentSignature(m_ID);
}

void Entity::Invalidate()
{
	m_ID = INVALID_ID;
}

EntityManager::BasicHierarchicalView::BasicHierarchicalView(EntityManager* p_entityManager):
m_entityManager(p_entityManager)
{
	if (m_entityManager->m_isHierarchicalEntitiesDirty)
		UpdateHierarchicalEntites();
}

void EntityManager::BasicHierarchicalView::UpdateHierarchicalEntites()
{
	auto& hierrarchicalEntities = m_entityManager->m_hierarchicalEntites;
	for (auto& element : hierrarchicalEntities)
	{
		auto& index = element.first;
		auto entity = element.second;

		index = RecursiveHierarchyCreation(entity, 0);
	}

	//Sort all entities form root to leafs
	std::sort(hierrarchicalEntities.begin(), hierrarchicalEntities.end(), [](const std::pair<int, Entity> a, const std::pair<int, Entity> b)->bool
		{
			return a.first < b.first;
		});

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
	Events::EventListener componentAddedListener;
	componentAddedListener.connect<&EntityManager::OnParentAdded>(this);
	Events::EventManager::GetInstance().AddListener(componentAddedListener, Events::ComponentAddedEvent<ParentComponent>::s_type);

	Events::EventListener componentRemovedListener;
	componentRemovedListener.connect<&EntityManager::OnParentRemoved>(this);
	Events::EventManager::GetInstance().AddListener(componentRemovedListener, Events::ComponentRemovedEvent<ParentComponent>::s_type);
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

void EntityManager::OnParentAdded(Events::EventPtr p_event)
{
	auto entity = std::static_pointer_cast<Events::ComponentAddedEvent<ParentComponent>>(p_event)->m_entity;
	m_hierarchicalEntites.push_back(std::make_pair(0, entity));
	m_isHierarchicalEntitiesDirty = true;
}

void EntityManager::OnParentRemoved(Events::EventPtr p_event)
{
	auto entity = std::static_pointer_cast<Events::ComponentAddedEvent<ParentComponent>>(p_event)->m_entity;
	auto foundEntity = std::find_if(m_hierarchicalEntites.begin(), m_hierarchicalEntites.end(), [&entity](std::pair<int, Entity> p_element) -> bool
		{
			return std::get<1>(p_element).GetID() == entity.GetID();
		});

	if (foundEntity != m_hierarchicalEntites.end())
	{
		m_hierarchicalEntites.erase(foundEntity);
		m_isHierarchicalEntitiesDirty = true;
	}
}
