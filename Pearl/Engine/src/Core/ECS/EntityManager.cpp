#include"Core/Common/pearl_pch.h"

#include "Core/ECS/EntityManager.h"

using namespace PrCore::ECS;

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
