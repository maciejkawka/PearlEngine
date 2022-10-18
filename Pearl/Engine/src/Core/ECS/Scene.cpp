#include"Core/Common/pearl_pch.h"

#include "Core/ECS/Scene.h"
#include"Core/ECS/SystemManager.h"
#include"Core/ECS/Components.h"
#include"Core/ECS/Systems/MeshRendererSystem.h"
#include"Core/ECS/Systems/TransformSystem.h"

using namespace PrCore::ECS;

Scene::Scene(const std::string& p_name) :
	m_name(p_name)
{
	m_path = "";
	m_UUID = Utils::UUIDGenerator().Generate();

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
	p_entity.AddComponent<ToDestoryTag>();
}

void Scene::DestoryEntityImmediate(Entity p_entity)
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

	return Entity();
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

	return Entity();
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

	return Entity();
}

void Scene::OnEnable() const
{
	m_systemManager->UpdateOnEnable();
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

void Scene::CleanDestroyedEntities() const
{
	for (auto entity : m_entityManager->GetAllHierrarchicalEntities())
	{
		if (entity.HasComponent<ToDestoryTag>())
			continue;

		auto parentComponent = entity.GetComponent<ParentComponent>();
		auto parent = parentComponent->parent;

		if (parent.HasComponent<ToDestoryTag>())
			entity.AddComponent<ToDestoryTag>();
	}

	for(auto entity: m_entityManager->GetEntitiesWithComponents<ToDestoryTag>())
		m_entityManager->DestoryEntity(entity.GetID());
}

void Scene::UpdateHierrarchicalEntities(float p_dt) const
{
	m_systemManager->UpdateSystem<HierarchyTransform>(p_dt);
}

void Scene::OnDisable() const
{
	m_systemManager->UpdateOnDisable();
}

void Scene::RenderUpdate(float p_dt) const
{
	m_systemManager->UpdateSystem<MeshRendererSystem>(p_dt);
}

size_t Scene::GetEntitiesCount()
{
	return m_entityManager->GetEntityCount();
}

void Scene::OnSerialize(Utils::JSON::json& p_serialized)
{
	using namespace Utils::JSON;

	//Possible Scene Settings here

	//

	//Scene Serialize
	p_serialized["name"] = m_name;
	p_serialized["path"] = m_path;
	p_serialized["UUID"] = m_UUID;
	p_serialized["entitiesCount"] = m_entityManager->GetEntityCount();

	Utils::JSON::json entitesJSON;
	m_entityManager->OnSerialize(entitesJSON);

	p_serialized["entities"] = entitesJSON;

	Utils::JSON::json systemsJSON;
	m_systemManager->OnSerialize(systemsJSON);
	p_serialized["systems"] = systemsJSON;
}

void Scene::OnDeserialize(const Utils::JSON::json& p_deserialized)
{
	//Possible Scene Settings here

	//

	//Scene Deserialize
	m_name = p_deserialized["name"];
	m_path = p_deserialized["path"];
	m_UUID = p_deserialized["UUID"];

	auto entitiesJSON = p_deserialized["entities"];
	m_entityManager->OnDeserialize(entitiesJSON);

	auto systemsJSON = p_deserialized["systems"];
	m_systemManager->OnDeserialize(systemsJSON);
}
