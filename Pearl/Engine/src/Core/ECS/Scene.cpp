#include"Core/Common/pearl_pch.h"

#include "Core/ECS/Scene.h"
#include"Core/ECS/SystemManager.h"
#include"Core/ECS/Components.h"

using namespace PrCore::ECS;

Scene::Scene(const std::string& p_name) :
	m_name(p_name)
{
	m_path = "";
	Utils::UUIDGenerator(m_UUID);

	m_entityManager = new EntityManager();
	m_systemManager = new SystemManager(m_entityManager);
}

Scene::~Scene()
{
	delete m_systemManager;
	delete m_entityManager;
}

Entity Scene::CreateEntity(const std::string& p_name)
{
	auto entity = m_entityManager->CreateEntity();

	auto UUID = entity.AddComponent<UUIDComponent>();
	UUID->UUID = Utils::UUIDGenerator().Generate();

	auto name = entity.AddComponent<NameComponent>();
	name->name = p_name;

	auto tag = entity.AddComponent<TagComponent>();
	tag->tag = "Untagged";

	return entity;
}

void Scene::DestoryEntity(Entity p_entity)
{
	m_entityManager->DestoryEntity(p_entity.GetID());
}

Entity Scene::GetEntityByName(const std::string& p_name)
{
	auto entityViewer = m_entityManager->GetEntitiesWithComponents<NameComponent>();
	for (auto entity : entityViewer)
	{
		auto name = entity.GetComponent<NameComponent>();
		if (name->name == p_name)
			return entity;
	}

	PRLOG_WARN("No entity name {0} found", p_name);
	return Entity(INVALID_ID, m_entityManager);
}

Entity Scene::GetEntityByID(Utils::UUID p_UUID)
{
	auto entityViewer = m_entityManager->GetEntitiesWithComponents<UUIDComponent>();
	for (auto entity : entityViewer)
	{
		auto UUID = entity.GetComponent<UUIDComponent>();
		if (UUID->UUID == p_UUID)
			return entity;
	}

	PRLOG_WARN("No entity UUID {0} found", p_UUID);
	return Entity(INVALID_ID, m_entityManager);
}

Entity Scene::GetEntityByTag(const std::string& p_tag)
{
	auto entityViewer = m_entityManager->GetEntitiesWithComponents<TagComponent>();
	for (auto entity : entityViewer)
	{
		auto tag = entity.GetComponent<TagComponent>();
		if (tag->tag == p_tag)
			return entity;
	}

	PRLOG_WARN("No entity tag {0} found", p_tag);
	return Entity(INVALID_ID, m_entityManager);
}

void Scene::Update(float p_dt) const
{
	m_systemManager->UpdateGroup(UpdateGroup::Update, p_dt);
}

void Scene::FixUpdate(float p_dt) const
{
	m_systemManager->UpdateGroup(UpdateGroup::FixUpdate, p_dt);
}

void Scene::LateUpdate(float p_dt) const
{
	m_systemManager->UpdateGroup(UpdateGroup::LateUpdate, p_dt);
}

size_t Scene::GetEntitiesCount()
{
	return m_entityManager->GetEntityCount();
}
