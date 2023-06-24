#pragma once
#include"Editor/Components/TestFeatures.h"

#include"Renderer/Core/Light.h"
#include"Renderer/Resources/Mesh.h"
#include"Renderer/Resources/Material.h"
#include"Core/Resources/ResourceLoader.h"
#include"Core/ECS/SceneManager.h"
#include"Core/ECS/Scene.h"
#include"Core/ECS/Systems/TestSystem.h"
#include"Core/ECS/Systems/TransformSystem.h"
#include"Core/ECS/Systems/MeshRendererSystem.h"
#include"Core/Utils/JSONParser.h"
#include"Core/Filesystem/FileSystem.h"

using namespace PrEditor::Components;

TestFeatures::TestFeatures()
{
	auto scene1 = PrCore::ECS::SceneManager::GetInstance().LoadScene("SceneECS.pearl");

	//Add Sphere
	auto sphereMaterial1 = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<PrRenderer::Resources::Material>("PBR_IR_ValuesSphere.mat");
	//sphereMaterial1->SetProperty("roughnessValue", 0.99f);
	//sphereMaterial1->SetProperty("metallicValue", 0.01f);
	//sphereMaterial1->SetProperty("albedoValue", PrCore::Math::vec4(1.0f, 0.65f, 0.0f, 0.6f));
	//sphereMaterial1->SetProperty("aoValue", 1.0f);
	sphereMaterial1->SetRenderType(PrRenderer::Resources::RenderType::Opaque);

	auto sphereMesh1 = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Sphere);
	//sphereMaterial1->SetProperty("albedoValue", PrCore::Math::vec4(1.0f, 0, 0, 0));


	for (int i = 0; i < 20; i++)
	{
		for (int j = 0; j < 20; j++)
		{
			auto sphere = scene1->CreateEntity("Sphere " + std::to_string(i) + "_" + std::to_string(j));
			auto spheretransform = sphere.AddComponent<PrCore::ECS::TransformComponent>();
			spheretransform->SetPosition(PrCore::Math::vec3(i * 5, j * 5, -10));
			spheretransform->SetLocalScale(PrCore::Math::vec3(2, 2, 2));
			auto meshRenderer1 = sphere.AddComponent<PrCore::ECS::MeshRendererComponent>();
			meshRenderer1->material = sphereMaterial1;
			meshRenderer1->mesh = sphereMesh1;
		}
	}

	return;

	auto scene = PrCore::ECS::SceneManager::GetInstance().CreateScene("TESTSCENE");
	scene->RegisterSystem<PrCore::ECS::HierarchyTransform>();
	scene->RegisterSystem<PrCore::ECS::MeshRendererSystem>();
	scene->RegisterSystem<PrCore::ECS::TestSystem>();

	//Add Camera
	auto camera = scene->CreateEntity("Camera");
	//camera.AddComponent<PrCore::ECS::CameraComponent>();
	camera.AddComponent<PrCore::ECS::TransformComponent>();

	//Add Lights
	auto light = scene->CreateEntity("Light1");
	auto lightComponent = light.AddComponent<PrCore::ECS::LightComponent>();
	lightComponent->SetType(PrCore::ECS::LightType::Point);
	lightComponent->SetColor({ 300, 300, 300 });
	auto transformComponent = light.AddComponent<PrCore::ECS::TransformComponent>();
	transformComponent->SetPosition({ 0, 2, 10 });

	light = scene->CreateEntity("Light2");
	lightComponent = light.AddComponent<PrCore::ECS::LightComponent>();
	lightComponent->SetType(PrCore::ECS::LightType::Point);
	lightComponent->SetColor({ 300, 300, 300 });
	transformComponent = light.AddComponent<PrCore::ECS::TransformComponent>();
	transformComponent->SetPosition({ 0, 2, 5 });

	light = scene->CreateEntity("Light3");
	lightComponent = light.AddComponent<PrCore::ECS::LightComponent>();
	lightComponent->SetType(PrCore::ECS::LightType::Point);
	lightComponent->SetColor({ 300, 300, 0 });
	transformComponent = light.AddComponent<PrCore::ECS::TransformComponent>();
	transformComponent->SetPosition({ 0, -2, 10 });

	light = scene->CreateEntity("Light4");
	lightComponent = light.AddComponent<PrCore::ECS::LightComponent>();
	lightComponent->SetType(PrCore::ECS::LightType::Point);
	lightComponent->SetColor({ 300, 0, 300 });
	transformComponent = light.AddComponent<PrCore::ECS::TransformComponent>();
	transformComponent->SetPosition({ 0, -2, 5 });

	//Add Sphere
	auto sphereMaterial = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<PrRenderer::Resources::Material>("PBR_IR_Values.mat");
	sphereMaterial->SetProperty("roughnessValue", 0.6f);
	sphereMaterial->SetProperty("metallicValue", 0.99f);
	sphereMaterial->SetProperty("albedoValue", PrCore::Math::vec3(1.0f, 0.65f, 0.0f));
	sphereMaterial->SetProperty("aoValue", 1.0f);

	auto sphereMesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Sphere);



	auto sphere = scene->CreateEntity("Sphere");
	auto spheretransform = sphere.AddComponent<PrCore::ECS::TransformComponent>();
	spheretransform->SetPosition(PrCore::Math::vec3(0, 5, -10));
	auto meshRenderer = sphere.AddComponent<PrCore::ECS::MeshRendererComponent>();

	meshRenderer->material = sphereMaterial;
	meshRenderer->mesh = sphereMesh;

	//Add Gun
	auto gun = scene->CreateEntity("Gun");
	auto gunTransform = gun.AddComponent<PrCore::ECS::TransformComponent>();
	auto gunMeshRenderer = gun.AddComponent<PrCore::ECS::MeshRendererComponent>();

	auto gunMesh = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<PrRenderer::Resources::Mesh>("yo.obj");
	auto gunMaterial = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<PrRenderer::Resources::Material>("PBR_IR_UniversalGun.mat");
	gunMaterial->SetProperty("normalMapping", true);
	gunMaterial->SetProperty("aoValue", 1.0f);

	gunTransform->SetPosition({ 0, 2, 10 });
	gunTransform->SetRotation(PrCore::Math::quat({ PrCore::Math::radians(0.0f), PrCore::Math::radians(0.f),PrCore::Math::radians(0.0f) }));
	gunTransform->SetLocalScale(PrCore::Math::vec3(0.05f));

	gunMeshRenderer->mesh = gunMesh;
	gunMeshRenderer->material = gunMaterial;

	//Add Gun
	auto pan = scene->CreateEntity("Pan");
	auto panTransform = pan.AddComponent<PrCore::ECS::TransformComponent>();
	auto panMeshRenderer = pan.AddComponent<PrCore::ECS::MeshRendererComponent>();

	auto panMesh = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<PrRenderer::Resources::Mesh>("pan.obj");
	auto panMaterial = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<PrRenderer::Resources::Material>("PBR_IR_UniversalPan.mat");
	panMaterial->SetProperty("normalMapping", true);
	panMaterial->SetProperty("aoValue", 1.0f);
	
	panTransform->SetPosition({ 0, 10, 10 });
	panTransform->SetRotation(PrCore::Math::quat({ PrCore::Math::radians(0.0f), PrCore::Math::radians(0.f),PrCore::Math::radians(0.0f) }));
	panTransform->SetLocalScale(PrCore::Math::vec3(0.5f));
	
	panMeshRenderer->mesh = panMesh;
	panMeshRenderer->material = panMaterial;

	PrCore::Utils::JSON::json json;
	scene->OnSerialize(json);
	auto dump = json.dump(4);
}

TestFeatures::~TestFeatures()
{}

void TestFeatures::Update(float p_deltaTime)
{
}
