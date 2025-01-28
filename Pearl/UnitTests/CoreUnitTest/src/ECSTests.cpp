#include <CommonUnitTest/Common/common.h>

#include "Core/Utils/StringUtils.h"
#include "Core/Threading/ThreadSystem.h"
#include "Core/Threading/JobSystem.h"
#include "Core/Utils/Logger.h"
#include "Core/ECS/ECS.h"
#include "Core/Utils/JSONParser.h"

using namespace PrCore::ECS;

class EcsSystemTest : public ::testing::Test {
public:
	static void SetUpTestSuite()
	{
		//This will be replaced with mocked versions in the future when I implement system localizer 
		PrCore::Utils::Logger::Init();
		PrCore::Threading::ThreadSystem::Init();
		PrCore::Threading::JobSystem::Init(8);
		PrCore::Events::EventManager::Init();
		PrCore::ECS::SceneManager::Init();

		auto workerNum = PrCore::Threading::JobSystem::GetInstance().GetWorkerNum();
		EXPECT_EQ(workerNum, 8);
	}

	static void TearDownTestSuite()
	{
		//This will be replaced with mocked versions in the future when I implement system localizer 
		PrCore::ECS::SceneManager::Terminate();
		PrCore::Events::EventManager::Terminate();
		PrCore::Threading::JobSystem::Terminate();
		PrCore::Threading::ThreadSystem::Terminate();
		PrCore::Utils::Logger::Terminate();
	}
};

class UnitTestComponent : public BaseComponent {
public:
	int updateCounter = 0;
	bool IsEnabled = true;

	void OnSerialize(PrCore::Utils::JSON::json& p_serialized) override
	{
	}

	void OnDeserialize(const PrCore::Utils::JSON::json& p_deserialized) override
	{
	}
};

class UnitTestSystem : public BaseSystem {
public:
	void OnUpdate(float p_dt) override
	{
		for (auto [entity] : m_entityViewer.AllEntities())
		{
			entity.GetComponent<UnitTestComponent>()->updateCounter++;
		}

		for (auto [entity, unitTestComponent] : m_entityViewer.EntitesWithComponents<UnitTestComponent>())
		{
			unitTestComponent->updateCounter++;
		}
	}

	void OnCreate() override
	{
	}

	void OnEnable() override
	{
		for (auto [entity, unitTestComponent] : m_entityViewer.EntitesWithComponents<UnitTestComponent>())
		{
			unitTestComponent->IsEnabled = true;
		}
	}

	void OnDisable() override
	{
		for (auto [entity, unitTestComponent] : m_entityViewer.EntitesWithComponents<UnitTestComponent>())
		{
			unitTestComponent->IsEnabled = false;
		}
	}

	void OnSerialize(PrCore::Utils::JSON::json& p_serialized) override
	{
	}

	void OnDeserialize(const PrCore::Utils::JSON::json& p_deserialized) override
	{
	}
};

class MTUnitTestSystem : public BaseSystem {
public:
	void OnUpdate(float p_dt) override
	{
		m_entityViewer.MT_AllEntities([](Entity entity)
			{
				entity.GetComponent<UnitTestComponent>()->updateCounter++;
			});

		m_entityViewer.MT_EntitesWithComponents<UnitTestComponent>([](Entity entity, auto unitTestComponent)
			{
				unitTestComponent->updateCounter++;
			});
	}

	void OnCreate() override
	{
	}

	void OnEnable() override
	{
	}

	void OnDisable() override
	{
	}

	void OnSerialize(PrCore::Utils::JSON::json& p_serialized) override
	{
	}

	void OnDeserialize(const PrCore::Utils::JSON::json& p_deserialized) override
	{
	}
};

class LateUpdate : public BaseSystem {
public:

	LateUpdate()
	{
		m_updateGroup = (uint8_t)UpdateGroup::LateUpdate;
	}

	void OnUpdate(float p_dt) override
	{
		m_entityViewer.MT_AllEntities([](Entity entity)
			{
				entity.GetComponent<UnitTestComponent>()->updateCounter += 10;
			});
	}

	void OnCreate() override
	{
	}

	void OnEnable() override
	{
	}

	void OnDisable() override
	{
	}

	void OnSerialize(PrCore::Utils::JSON::json& p_serialized) override
	{
	}

	void OnDeserialize(const PrCore::Utils::JSON::json& p_deserialized) override
	{
	}
};

TEST_F(EcsSystemTest, EntityModifications)
{
	auto sceneManager = PrCore::ECS::SceneManager::GetInstancePtr();
	auto scene = sceneManager->CreateScene("TestScene");

	EXPECT_STREQ(scene->GetSceneName().c_str(), "TestScene");

	for (int i = 0; i < 5; i++)
	{
		auto entityName = "Entity" + PrCore::StringUtils::ToString(i);
		auto entity = scene->CreateEntity(entityName);

		entity.AddComponent<UnitTestComponent>();
		EXPECT_TRUE(entity.HasComponent<UnitTestComponent>());

		auto name = entity.GetComponent<NameComponent>();
		EXPECT_STREQ(name->name.c_str(), entityName.c_str());

		auto tag = entity.GetComponent<TagComponent>();
		EXPECT_STREQ(tag->tag.c_str(), "Untagged");
	}

	EXPECT_EQ(scene->GetEntitiesCount(), 5);

	auto entity = scene->GetEntityByName("Entity4");
	EXPECT_TRUE(entity.IsValid());
	EXPECT_STREQ(entity.GetComponent<NameComponent>()->name.c_str(), "Entity4");
	
	auto ID = entity.GetComponent<UUIDComponent>()->UUID;
	auto entityById = scene->GetEntityByID(ID);
	EXPECT_TRUE(entity == entityById);

	entity.RemoveComponent<UnitTestComponent>();
	EXPECT_FALSE(entity.HasComponent<UnitTestComponent>());

	EXPECT_TRUE(entity.GetComponentSignature().test(0)); // ID Component
	EXPECT_TRUE(entity.GetComponentSignature().test(1)); // Name Component
	EXPECT_TRUE(entity.GetComponentSignature().test(2)); // Tag Component
	EXPECT_FALSE(entity.GetComponentSignature().test(3)); // Removed UnitTest Component

	scene->DestoryEntity(scene->GetEntityByName("Entity0"));
	scene->DestoryEntity(scene->GetEntityByName("Entity1"));
	scene->DestoryEntity(scene->GetEntityByName("Entity2"));
	scene->DestoryEntity(scene->GetEntityByName("Entity3"));
	scene->DestoryEntity(scene->GetEntityByName("Entity4"));

	scene->CleanDestroyedEntities();

	EXPECT_EQ(scene->GetEntitiesCount(),  0);

	EXPECT_FALSE(scene->GetEntityByName("Entity0").IsValid());
	EXPECT_FALSE(scene->GetEntityByName("Entity1").IsValid());
	EXPECT_FALSE(scene->GetEntityByName("Entity2").IsValid());
	EXPECT_FALSE(scene->GetEntityByName("Entity3").IsValid());
	EXPECT_FALSE(scene->GetEntityByName("Entity4").IsValid());

	sceneManager->DeleteScene(scene);
}

TEST_F(EcsSystemTest, SystemUpdate)
{
	auto sceneManager = PrCore::ECS::SceneManager::GetInstancePtr();
	auto scene = sceneManager->CreateScene("TestScene");

	scene->RegisterSystem<UnitTestSystem>();
	scene->RegisterSystem<MTUnitTestSystem>();

	std::vector<Entity> entities;
	for (int i = 0; i < 5; i++)
	{
		auto entityName = "Entity" + PrCore::StringUtils::ToString(i);
		auto entity = scene->CreateEntity(entityName);
		entity.AddComponent<UnitTestComponent>();
		entities.push_back(entity);
	}

	// Update the system
	scene->Update(0);
	for (auto entity : entities)
	{
		entity.GetComponent<UnitTestComponent>()->updateCounter = 4;
		entity.GetComponent<UnitTestComponent>()->IsEnabled = true;
	}

	// Test Disable/Enable System
	scene->SetActiveSystem<UnitTestSystem>(false);
	auto isActive = scene->IsActiveSystem<UnitTestSystem>();

	scene->OnDisable();

	EXPECT_FALSE(isActive);
	for (auto entity : entities)
	{
		EXPECT_FALSE(entity.GetComponent<UnitTestComponent>()->IsEnabled);
	}

	scene->SetActiveSystem<UnitTestSystem>(true);
	EXPECT_TRUE(scene->IsActiveSystem<UnitTestSystem>());

	scene->OnEnable();

	for (auto entity : entities)
	{
		EXPECT_TRUE(entity.GetComponent<UnitTestComponent>()->IsEnabled);
	}

	// Test LateUpdate
	scene->RegisterSystem<LateUpdate>();
	scene->Update(0);
	for (auto entity : entities)
	{
		EXPECT_EQ(entity.GetComponent<UnitTestComponent>()->updateCounter, 8);
	}

	scene->LateUpdate(0);
	for (auto entity : entities)
	{
		EXPECT_EQ(entity.GetComponent<UnitTestComponent>()->updateCounter, 18);
	}

	// Cleanup
	sceneManager->DeleteScene(scene);
}

TEST_F(EcsSystemTest, HierrarchicalEntities)
{
	auto sceneManager = PrCore::ECS::SceneManager::GetInstancePtr();
	auto scene = sceneManager->CreateScene("TestScene");
	scene->RegisterSystem<HierarchyTransform>();

	auto root = scene->CreateEntity("Root");
	root.AddComponent<TransformComponent>();

	// Add children and parents to the scene
	std::vector<Entity> parentEntities;
	std::vector<Entity> childrenEntities;
	for (int i = 0; i < 100; i++)
	{
		auto parentEntity = scene->CreateEntity("ParentEntuty");

		auto transform = parentEntity.AddComponent<PrCore::ECS::TransformComponent>();
		auto position = PrCore::Math::vec3(0, 0, 0);
		transform->SetPosition(position);
		transform->SetLocalScale(PrCore::Math::vec3{ 1 });

		parentEntity.AddComponent<PrCore::ECS::ParentComponent>()->SetParent(root);
		parentEntities.push_back(parentEntity);

		for (int j = 0; j < 80; j++)
		{
			auto childEntity = scene->CreateEntity("ChildEntity");
			auto transform = childEntity.AddComponent<PrCore::ECS::TransformComponent>();
			auto position = PrCore::Math::vec3(1, 2, 3);
			transform->SetPosition(position);
			transform->SetLocalScale(PrCore::Math::vec3{ 10 });

			auto parent = childEntity.AddComponent<PrCore::ECS::ParentComponent>();
			parent->parent = parentEntity;

			childrenEntities.push_back(childEntity);
		}
	}


	// Update all hierarchical entities
	for (int i = 0; i < 10; i++)
	{
		root.GetComponent<TransformComponent>()->SetPosition({ i,i,i });
		scene->UpdateHierrarchicalEntities(0);

		for (auto entity : parentEntities)
		{
			auto pos = entity.GetComponent<TransformComponent>()->GetPosition();
			EXPECT_EQ(pos.x, i);
			EXPECT_EQ(pos.y, i);
			EXPECT_EQ(pos.z, i);
		}

		for (auto entity : childrenEntities)
		{
			auto pos = entity.GetComponent<TransformComponent>()->GetPosition();
			EXPECT_EQ(pos.x, i + 1.0f);
			EXPECT_EQ(pos.y, i + 2.0f);
			EXPECT_EQ(pos.z, i + 3.0f);
		}
	}


	// change children to point the root
	for (auto entity : parentEntities)
	{
		entity.GetComponent<ParentComponent>()->SetParent(Entity());
	}

	for (auto entity : childrenEntities)
	{
		entity.GetComponent<ParentComponent>()->SetParent(root);
	}

	root.GetComponent<TransformComponent>()->SetPosition({ 20, 20, 20 });
	scene->UpdateHierrarchicalEntities(0);

	for (auto entity : parentEntities)
	{
		auto pos = entity.GetComponent<TransformComponent>()->GetPosition();
		EXPECT_EQ(pos.x, 9);
		EXPECT_EQ(pos.y, 9);
		EXPECT_EQ(pos.z, 9);
	}

	for (auto entity : childrenEntities)
	{
		auto pos = entity.GetComponent<TransformComponent>()->GetPosition();
		EXPECT_EQ(pos.x, 21.0f);
		EXPECT_EQ(pos.y, 22.0f);
		EXPECT_EQ(pos.z, 23.0f);
	}


	// check if deleting root removes all children
	scene->DestoryEntity(root);
	scene->CleanDestroyedEntities();
	scene->UpdateHierrarchicalEntities(0);
	EXPECT_EQ(scene->GetEntitiesCount(), 100);


	// no hierarchical entities
	for (auto entity : parentEntities)
	{
		entity.RemoveComponent<ParentComponent>();
	}
	scene->UpdateHierrarchicalEntities(0);


	// cleanup
	sceneManager->DeleteScene(scene);
}