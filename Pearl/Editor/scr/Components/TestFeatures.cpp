#include"Editor/Components/TestFeatures.h"

#include"Core/Input/InputManager.h"

#include"Renderer/Core/Light.h"
#include"Renderer/Resources/Mesh.h"
#include"Renderer/Resources/Cubemap.h"
#include"Renderer/Resources/Material.h"
#include"Renderer/Resources/Texture2D.h"
#include"Core/Resources/ResourceLoader.h"
#include"Core/ECS/SceneManager.h"
#include"Core/ECS/Systems/TransformSystem.h"
#include"Core/ECS/Test.h"

using namespace PrEditor::Components;

TestFeatures::TestFeatures()
{
	auto scene = PrCore::ECS::SceneManager::GetInstance().CreateScene("TESTSCENE");
	scene->RegisterSystem<PrCore::ECS::HierarchyTransform>();
	scene->RegisterSystem<PrCore::ECS::MeshRendererSystem>();

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
	sphereMaterial->SetProperty("albedoValue", PrCore::Math::vec3(1.0f, 0.65f, 0.0f));
	sphereMaterial->SetProperty("aoValue", 1.0f);

	auto sphereMesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Sphere);


	auto sphere = scene->CreateEntity("Sphere");
	auto spheretransform = sphere.AddComponent<PrCore::ECS::TransformComponent>();
	auto meshRenderer = sphere.AddComponent<PrCore::ECS::MeshRendererComponent>();

	spheretransform->SetPosition({ 0,0,0 });
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
}

TestFeatures::~TestFeatures()
{}

void TestFeatures::Update(float p_deltaTime)
{
}
