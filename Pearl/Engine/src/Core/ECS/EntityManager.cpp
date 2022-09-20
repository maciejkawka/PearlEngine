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

	m_entityManager->DestoryEntity(m_ID);
	Invalidate();
}

bool Entity::IsValid() const
{
	PR_ASSERT(m_entityManager != nullptr, "EntityManager is nullptr");

	return m_entityManager->IsValid(m_ID);
}

void Entity::Invalidate()
{
	m_ID = INVALID_ID;
}


EntityManager::EntityManager():
	m_entitiesNumber(0)
{
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
	for (const auto& componentPoolPair : m_ComponentPools)
	{
		//Small  if statement to faster find if component exist
		if(m_entitiesSignature[p_ID.GetIndex() -1].test(componentPoolPair.first))
			componentPoolPair.second->EntityDestroyed(p_ID);
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
