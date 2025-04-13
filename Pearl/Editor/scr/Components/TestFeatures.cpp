#include"Editor/Components/TestFeatures.h"

#include"Editor/Assets/Model/ModelEntityGraph.h"
#include"Editor/Assets/Model/ModelResourceLoader.h"
#include"Editor/Assets/Model/ModelResource.h"

#include"Renderer/Resources/Light.h"
#include"Renderer/Resources/Mesh.h"
#include"Renderer/Resources/Material.h"
#include"Core/Resources/ResourceSystem.h"
#include"Core/ECS/SceneManager.h"
#include"Core/ECS/Scene.h"
#include"Core/ECS/Systems/TestSystem.h"
#include"Core/ECS/Systems/TransformSystem.h"
#include"Core/ECS/Systems/MeshRendererSystem.h"
#include"Core/ECS/Systems/PhysicsUpdateSystem.h"
#include "Core/ECS/Components/PhysicsComponents.h"

// Temporary register loaders
#include "Core/Resources/ResourceSystem.h"
#include "Core/Resources/ResourceDatabase.h"

#include "Renderer/Resources/Shader.h"
#include "Renderer/Resources/Cubemap.h"
#include "Renderer/Resources/Material.h"
#include "Renderer/Resources/Mesh.h"
#include "Renderer/Resources/Texture.h"

#include "Renderer/Resources/BasicCubemapLoader.h"
#include "Renderer/Resources/HdrCubemapLoader.h"
#include "Renderer/Resources/MaterialLoader.h"
#include "Renderer/Resources/MeshOBJLoader.h"
#include "Renderer/Resources/ShaderLoader.h"
#include "Renderer/Resources/Texture2DLoader.h"
#include "Renderer/Resources/Light.h"

#include "Core/Threading/JobSystem.h"
#include "Core/Threading/ThreadSystem.h"

#include "Physics/Core/PhysicsSystem.h"
#include "Physics/Shape/IGeometry.h"
#include "Physics/Actor/IRigidBody.h"

//
using namespace PrEditor::Components;

PrPhysics::IRigidBodyDynamicPtr ptrActor;
PrPhysics::IShapePtr ptrShape;

std::vector<PrPhysics::IActorPtr> conteiner;

TestFeatures::TestFeatures()
{
	using namespace PrCore::Resources;
	using namespace PrRenderer::Resources;

	// Load Stress Test
	auto scene10 = PrCore::ECS::SceneManager::GetInstance().CreateScene("scene/render_stress_test.pearl");
	scene10->RegisterSystem<PrCore::ECS::HierarchyTransform>();
	scene10->RegisterSystem<PrCore::ECS::MeshRendererSystem>();
	scene10->RegisterSystem<PrCore::ECS::RenderStressTest>();
	scene10->RegisterSystem<PrCore::ECS::PhysicsUpdateSystem>();

	auto modeHandl = PrCore::Resources::ResourceSystem::GetInstance().Load<Assets::ModelResource>("Model/ocean.glb");
	modeHandl->AddEntitesToScene(scene10);

	auto root = scene10->GetEntityByName("ocean").GetComponent<PrCore::ECS::TransformComponent>();
	root->SetPosition(PrCore::Math::vec3{ 0,5.0f,0.0f });
	

	//Add physics
	auto physicsPtr = PrPhysics::PhysicsSystem::GetInstancePtr();

	PrPhysics::Material material;
	material.staticFriction = 0.0f;
	material.dynamicFriction = 0.1f;
	material.restitution = .1f;

	//Ocean
	{
		auto shape = physicsPtr->CreateShape(PrPhysics::PlaneGeometry{}, material);
		PrPhysics::Transform transform;
		transform.position = { 0, 5.0f, 0.0f };
		transform.rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(0, 0, 1));
		auto rigidBody = physicsPtr->CreateRigidStatic(transform);
		rigidBody->AttachShape(shape);
		physicsPtr->AddActor(rigidBody);

		auto rigidbodyComponent = scene10->GetEntityByName("ocean").AddComponent<PrCore::ECS::RigidBodyStaticComponent>();
		rigidbodyComponent->rigidBody = rigidBody;
	}

	
	for (int i = 0; i < 10; i += 2)
	{
		for (int j = 0; j < 10; j += 2)
		{	
			auto entity = scene10->CreateEntity("PhysicsBox");
			auto physcomponent = entity.AddComponent<PrCore::ECS::RigidBodyDynamicComponent>();
			
			auto rigidBody = physcomponent->rigidBody;
			auto shape = physicsPtr->CreateShape(PrPhysics::BoxGeometery{ 0.5f, 0.5f, 0.5f }, material);
			rigidBody->AttachShape(shape);

			auto logoTransform = entity.AddComponent<PrCore::ECS::TransformComponent>();
			auto logoMesh = entity.AddComponent<PrCore::ECS::MeshRendererComponent>();

			logoTransform->SetPosition(PrCore::Math::vec3({ 15, 25.0f + i * 3, j * 2 }));
			logoTransform->SetLocalScale(PrCore::Math::vec3(1.0f));
			logoMesh->mainMaterial = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Material>("stress_test/logo.mat");
			logoMesh->mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Cube);
		}
	}

	for (int i = 0; i < 10; i += 2)
	{
		for (int j = 0; j < 10; j += 2)
		{			
			auto entity = scene10->CreateEntity("PhysicsBox");
			auto physcomponent = entity.AddComponent<PrCore::ECS::RigidBodyDynamicComponent>();

			auto rigidBody = physcomponent->rigidBody;
			auto shape = physicsPtr->CreateShape(PrPhysics::SphereGeometry{ 0.5f }, material);
			rigidBody->AttachShape(shape);

			auto logoTransform = entity.AddComponent<PrCore::ECS::TransformComponent>();
			auto logoMesh = entity.AddComponent<PrCore::ECS::MeshRendererComponent>();
			logoTransform->SetPosition(PrCore::Math::vec3({ -10, 25.0f + i * 3, j * 2 }));
			logoTransform->SetLocalScale(PrCore::Math::vec3(1.0f));
			logoMesh->mainMaterial = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Material>("stress_test/emissionCapsule.mat");
			logoMesh->mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Sphere);
		}
	}

	// Physics Test
	//auto physicsPtr = PrPhysics::PhysicsSystem::GetInstancePtr();
	//PrPhysics::Material material;
	//material.dynamicFriction = 1000.0f;
	//material.staticFriction = 1000.0f;

	//for(int i=0;i<10;i+=2)
	//{
	//	auto shape = physicsPtr->CreateShape(PrPhysics::BoxGeometery{ 1.0f, 1.0f, 1.0f }, material);
	//	PrPhysics::Transform transform;
	//	transform.position = { 2,i*5,2 };
	//	auto rigidBody = physicsPtr->CreateRigidDynamic(transform);
	//	rigidBody->AttachShape(shape);
	//	physicsPtr->AddActor(rigidBody);

	//	conteiner.push_back(rigidBody);

	//	ptrActor = rigidBody;
	//	ptrShape = shape;
	//}

	//material.restitution = 1.0f;

	//for (int i = 0; i < 100; i += 2)
	//{
	//	auto shape = physicsPtr->CreateShape(PrPhysics::SphereGeometry{ 1.0f}, material);
	//	PrPhysics::Transform transform;
	//	transform.position = { 6,i * 5,i*2 };
	//	auto rigidBody = physicsPtr->CreateRigidDynamic(transform);
	//	rigidBody->AttachShape(shape);
	//	physicsPtr->AddActor(rigidBody);

	//	conteiner.push_back(rigidBody);

	//	ptrActor = rigidBody;
	//	ptrShape = shape;
	//}

	//material.staticFriction = 0.0f;
	//material.dynamicFriction = 0.0f;
	//material.restitution = 0.0f;
	//auto shape = physicsPtr->CreateShape(PrPhysics::PlaneGeometry{}, material);
	//PrPhysics::Transform transform;
	//transform.position = { 0, 0, 0 };
	//transform.rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(0, 0, 1));
	//auto rigidBody = physicsPtr->CreateRigidStatic(transform);
	//rigidBody->AttachShape(shape);
	//physicsPtr->AddActor(rigidBody);

	//conteiner.push_back(rigidBody);
}

TestFeatures::~TestFeatures()
{
	conteiner.clear();
	ptrActor.reset();
	ptrShape.reset();
}

void TestFeatures::Update(float p_deltaTime)
{
}
