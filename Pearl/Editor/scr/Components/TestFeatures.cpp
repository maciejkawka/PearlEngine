#include"Editor/Components/TestFeatures.h"

#include"Renderer/Resources/Light.h"
#include"Renderer/Resources/Mesh.h"
#include"Renderer/Resources/Material.h"
#include"Core/Resources/ResourceSystem.h"
#include"Core/ECS/SceneManager.h"
#include"Core/ECS/Scene.h"
#include"Core/ECS/Systems/TestSystem.h"
#include"Core/ECS/Systems/TransformSystem.h"
#include"Core/ECS/Systems/MeshRendererSystem.h"
#include"Core/Utils/JSONParser.h"
#include"Core/Filesystem/FileSystem.h"
#include"Core/Utils/PathUtils.h"

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
//
using namespace PrEditor::Components;

TestFeatures::TestFeatures()
{
	using namespace PrCore::Resources;
	using namespace PrRenderer::Resources;

	//Register resources
	//{
	//	auto textureDatabase = std::make_unique<ResourceDatabase>();
	//	textureDatabase->RegisterLoader(".png", std::make_unique<Texture2DLoader>());
	//	textureDatabase->RegisterLoader(".jpg", std::make_unique<Texture2DLoader>());
	//	textureDatabase->RegisterLoader(".tga", std::make_unique<Texture2DLoader>());
	//	textureDatabase->RegisterLoader(".hdr", std::make_unique<Texture2DLoader>());
	//	ResourceSystem::GetInstance().RegisterDatabase<Texture2Dv2>(std::move(textureDatabase));

	//	auto materialLoader = std::make_unique<ResourceDatabase>();
	//	materialLoader->RegisterLoader(".mat", std::make_unique<MaterialLoader>());
	//	ResourceSystem::GetInstance().RegisterDatabase<Materialv2>(std::move(materialLoader));

	//	auto shaderLoader = std::make_unique<ResourceDatabase>();
	//	shaderLoader->RegisterLoader(".shader", std::make_unique<ShaderLoader>());
	//	ResourceSystem::GetInstance().RegisterDatabase<Shaderv2>(std::move(shaderLoader));

	//	auto meshLoader = std::make_unique<ResourceDatabase>();
	//	meshLoader->RegisterLoader(".obj", std::make_unique<MeshOBJLoader>());
	//	ResourceSystem::GetInstance().RegisterDatabase<Meshv2>(std::move(meshLoader));

	//	auto cubemapDatabase = std::make_unique<ResourceDatabase>();
	//	cubemapDatabase->RegisterLoader(".cubemap", std::make_unique<BasicCubemapLoader>());
	//	cubemapDatabase->RegisterLoader(".hdr", std::make_unique<HdrCubemapLoader>());
	//	ResourceSystem::GetInstance().RegisterDatabase<Cubemapv2>(std::move(cubemapDatabase));
	//}

	//auto logoMat = ResourceSystem::GetInstance().Load<Materialv2>("StressTest/logo.mat");
	//auto meshResourceData = ResourceSystem::GetInstance().Load<Meshv2>("sphere_lowPoly.obj");
	//auto newTex = ResourceSystem::GetInstance().Load<Texture2Dv2>("emission.png");
	//auto newShader = ResourceSystem::GetInstance().Load<Shaderv2>("IrradianceMap.shader");
	//auto cubemap = ResourceSystem::GetInstance().Load<Cubemapv2>("SkyBox/skybox.cubemap");
	//auto cubemapHdr = ResourceSystem::GetInstance().Load<Cubemapv2>("SkyBox/UnearthlyRed4k.hdr");

	//auto elow = newTex->GetWidth();
	//auto data = newTex.GetData();
	//auto eloh = newTex->GetHeight();
	//auto exist = newTex != nullptr;
	//auto isnull = newTex == nullptr;
	//ResourceSystem::GetInstance().Unload<Texture2Dv2>("emission.png");
	//exist = newTex != nullptr;
	//isnull = newTex == nullptr;

	// Load Stress Test
	auto scene10 = PrCore::ECS::SceneManager::GetInstance().LoadScene("RenderStressTest.pearl");

	//auto parentEntity = scene10->GetEntityByName("Light0");
	//for (int i = 1; i < 56; i++)
	//{
	//	auto entity = scene10->GetEntityByName("Light" + std::to_string(i));
	//	auto parent = entity.AddComponent<PrCore::ECS::ParentComponent>();
	//	auto transform = entity.GetComponent<PrCore::ECS::TransformComponent>();
	//	auto vec = transform->GetLocalPosition();
	//	vec.y = 0.0f;
	//	transform->SetLocalPosition(vec);

	//	parent->parent = parentEntity;
	//}

	//std::string elo{ "Folder/File.ext" };
	//auto returnThat = PrCore::PathUtils::GetFilename(elo);
	//auto returnThatView = PrCore::PathUtils::GetFilenameInPlace(elo);

	//auto path = PrCore::PathUtils::MakePath("Directory", "FileName", "txt");
	//auto pat1 = PrCore::PathUtils::MakePath("Directory", "FileName.txt");

	//auto newPath = PrCore::PathUtils::ReplaceExtension("Elo/Elo1/Elo2/FileName.txt", "png");
	//auto newPath1 = PrCore::PathUtils::ReplaceExtension("FileName.txt", "png");
	//newPath1 = PrCore::PathUtils::ReplaceExtension("FileName", "png");
	//newPath1 = PrCore::PathUtils::ReplaceExtension("FileName", "..png");
	//newPath1 = PrCore::PathUtils::ReplaceExtension("FileName..", "..png");
	//newPath1 = PrCore::PathUtils::ReplaceExtension("FileName..txt", "..png");

	//auto noExtension = PrCore::PathUtils::RemoveExtension("Directory/FileName.ext");
	//auto Newpath = "C:ELO1/ELO0/FileName.elo";
	//PrCore::PathUtils::IsAbsolute(Newpath);
	//PrCore::PathUtils::IsRelative(Newpath);

	//auto noExtension1 = PrCore::PathUtils::RemoveExtensionInPlace(Newpath);

	//auto pathNewName = PrCore::PathUtils::ReplaceFilename(Newpath, "NewName");

	//auto vec = PrCore::PathUtils::SplitPath(Newpath);
	//auto vec1 = PrCore::PathUtils::SplitPathInPlace(Newpath);

	//auto generation = PrCore::PathUtils::GetSubFolder(Newpath, 2);
	//auto generation1 = PrCore::PathUtils::GetSubFolderInPlace(Newpath, 2);

	// Walls
	auto logoEntity = scene10->CreateEntity("Logo1");
	auto logoTransform = logoEntity.AddComponent<PrCore::ECS::TransformComponent>();
	auto logoMesh = logoEntity.AddComponent<PrCore::ECS::MeshRendererComponent>();
	logoTransform->SetPosition(PrCore::Math::vec3(20, 5.0f, -20));
	logoTransform->SetRotation(PrCore::Math::quat(PrCore::Math::radians(PrCore::Math::vec3(0, 0, 180))));
	logoTransform->SetLocalScale(PrCore::Math::vec3(5, 5, 1.1f));
	logoMesh->material = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Material>("StressTest/logo.mat");
	logoMesh->mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Cube);

	logoEntity = scene10->CreateEntity("Logo2");
	logoTransform = logoEntity.AddComponent<PrCore::ECS::TransformComponent>();
	logoMesh = logoEntity.AddComponent<PrCore::ECS::MeshRendererComponent>();
	logoTransform->SetPosition(PrCore::Math::vec3(20, 5.0f, 60));
	logoTransform->SetRotation(PrCore::Math::quat(PrCore::Math::radians(PrCore::Math::vec3(0, 0, 0))));
	logoTransform->SetLocalScale(PrCore::Math::vec3(5, 5, 1.1));
	logoMesh->material = PrCore::Resources::ResourceSystem::GetInstance().Get<PrRenderer::Resources::Material>("StressTest/logo.mat");
	logoMesh->mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Cube);

	//logoEntity = scene10->CreateEntity("Logo3");
	//logoTransform = logoEntity.AddComponent<PrCore::ECS::TransformComponent>();
	//logoMesh = logoEntity.AddComponent<PrCore::ECS::MeshRendererComponent>();
	//logoTransform->SetPosition(PrCore::Math::vec3(-20, 5.0f, 20));
	//logoTransform->SetRotation(PrCore::Math::quat(PrCore::Math::radians(PrCore::Math::vec3(0, 90, 0))));
	//logoTransform->SetLocalScale(PrCore::Math::vec3(5, 5, 3));
	//logoMesh->material = PrCore::Resources::ResourceSystem::GetInstance().Get<PrRenderer::Resources::Material>("StressTest/logo.mat");
	//logoMesh->mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Cube);

	logoEntity = scene10->CreateEntity("Logo4");
	logoTransform = logoEntity.AddComponent<PrCore::ECS::TransformComponent>();
	logoMesh = logoEntity.AddComponent<PrCore::ECS::MeshRendererComponent>();
	logoTransform->SetPosition(PrCore::Math::vec3(60, 5.0f, 20));
	logoTransform->SetRotation(PrCore::Math::quat(PrCore::Math::radians(PrCore::Math::vec3(0, 90, 0))));
	logoTransform->SetLocalScale(PrCore::Math::vec3(5, 5, 1.1));
	logoMesh->material = PrCore::Resources::ResourceSystem::GetInstance().Get<PrRenderer::Resources::Material>("StressTest/logo.mat");
	logoMesh->mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Cube);

	return;

	auto scene = PrCore::ECS::SceneManager::GetInstance().CreateScene("RenderStressTest");
	scene->RegisterSystem<PrCore::ECS::MeshRendererSystem>();
	scene->RegisterSystem<PrCore::ECS::RenderStressTest>();

	auto sphereMesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Sphere);
	auto unlightmaterial = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Material>("StressTest/light.mat");

	int lightCounter = 0;
	// Point Lights
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			auto entity = scene->CreateEntity("Light" + std::to_string(lightCounter));
			auto lightComponent = entity.AddComponent<PrCore::ECS::LightComponent>();
			auto transformComponent = entity.AddComponent<PrCore::ECS::TransformComponent>();
			auto meshComponent = entity.AddComponent<PrCore::ECS::MeshRendererComponent>();

			transformComponent->SetPosition(PrCore::Math::vec3(i * 5, 5, j * 5));
			transformComponent->SetLocalScale(PrCore::Math::vec3(1, 1, 1));

			lightComponent->m_light->SetType(PrRenderer::Resources::LightType::Point);
			lightComponent->m_light->SetRange(60.0f);
			lightComponent->m_shadowCast = lightCounter < 18 ? true : false;

			meshComponent->material = unlightmaterial;
			meshComponent->mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Sphere);
			meshComponent->shadowMesh = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Mesh>("sphere_lowPoly.obj");
			meshComponent->shadowCaster = false;

			lightCounter++;
		}
	}

	// Spot Lights
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			auto entity = scene->CreateEntity("Light" + std::to_string(lightCounter));
			auto lightComponent = entity.AddComponent<PrCore::ECS::LightComponent>();
			auto transformComponent = entity.AddComponent<PrCore::ECS::TransformComponent>();
			auto meshComponent = entity.AddComponent<PrCore::ECS::MeshRendererComponent>();

			transformComponent->SetPosition(PrCore::Math::vec3(-15 + i * 5, 5, j * 5));
			transformComponent->SetRotation(PrCore::Math::quat(PrCore::Math::radians(PrCore::Math::vec3(90.0f, 0.0f, 0.0f))));
			transformComponent->SetLocalScale(PrCore::Math::vec3(1, 1, 1));

			lightComponent->m_light->SetType(PrRenderer::Resources::LightType::Spot);
			lightComponent->m_light->SetInnerCone(20.0f);
			lightComponent->m_light->SetRange(60.0f);
			lightComponent->m_light->SetOutterCone(25.0f);
			lightComponent->m_shadowCast = true;

			meshComponent->mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Cube);
			meshComponent->material = unlightmaterial;
			meshComponent->shadowCaster = false;

			lightCounter++;
		}
	}

	{
		auto mainLight = scene->CreateEntity("MainLight");
		auto lightComponent = mainLight.AddComponent<PrCore::ECS::LightComponent>();
		auto transformLight = mainLight.AddComponent<PrCore::ECS::TransformComponent>();
		transformLight->SetRotation(PrCore::Math::quat(PrCore::Math::radians(PrCore::Math::vec3(45.0f, -100.0f, 0.0f))));
		PrRenderer::Resources::LightPtr light = std::make_shared<PrRenderer::Resources::Light>();
		light->SetColor({ 0.8f,0.8f,.8f,1.0f });
		light->SetType(PrRenderer::Resources::LightType::Directional);
		lightComponent->m_light = light;
		lightComponent->mainDirectLight = true;
		lightComponent->m_shadowCast = true;
	}

	// Small capsules to see shadows
	auto capsuleMaterial = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Material>("StressTest/capsule.mat");
	for (int i = 0; i < 20; i++)
	{
		for (int j = 0; j < 40; j++)
		{
			auto capsule = scene->CreateEntity("Ocluder " + std::to_string(i) + "_" + std::to_string(j));
			auto transform = capsule.AddComponent<PrCore::ECS::TransformComponent>();
			auto mesh = capsule.AddComponent<PrCore::ECS::MeshRendererComponent>();

			transform->SetPosition(PrCore::Math::vec3(i * 2 - 20.0f, 2.0f, j * 2 - 20.0));
			transform->SetLocalScale(PrCore::Math::vec3(0.5f));
			mesh->material = capsuleMaterial;
			mesh->mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Capsule);
			mesh->shadowMesh = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Mesh>("capsule_lowPoly.obj");
		}
	}

	// Overhead small objects
	auto overheadObjectMat = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Material>("StressTest/overheadSphere.mat");
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			auto object = scene->CreateEntity("Overhead " + std::to_string(i) + "_" + std::to_string(j));
			auto transform = object.AddComponent<PrCore::ECS::TransformComponent>();
			auto mesh = object.AddComponent<PrCore::ECS::MeshRendererComponent>();

			transform->SetPosition(PrCore::Math::vec3(i * 8 - 20.0f, 10.0f, j * 8 - 20.0));
			transform->SetLocalScale(PrCore::Math::vec3(0.5f));
			mesh->material = overheadObjectMat;
			mesh->shadowMesh = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Mesh>("sphere_lowPoly.obj");
			mesh->mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Sphere);
		}
	}

	// Emission Capsules
	//Create a new material
	auto capsuleMaterialEmission = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Material>("StressTest/emissionCapsule.mat");
	for (int i = 0; i < 12; i++)
	{
		auto capsule = scene->CreateEntity("Emission " + std::to_string(i));
		auto transform = capsule.AddComponent<PrCore::ECS::TransformComponent>();
		auto mesh = capsule.AddComponent<PrCore::ECS::MeshRendererComponent>();

		transform->SetPosition(PrCore::Math::vec3(50, 2.0f, -10 + i * 5));
		transform->SetLocalScale(PrCore::Math::vec3(1.f));
		transform->SetRotation(PrCore::Math::quat(PrCore::Math::radians(PrCore::Math::vec3(i * 20.0f, 180.0f, i * 10.0f))));

		mesh->material = capsuleMaterialEmission;
		mesh->mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Capsule);
		mesh->shadowMesh = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Mesh>("capsule_lowPoly.obj");
	}


	//Quad to cast shadow
	auto quad = scene->CreateEntity("Quad");
	auto quadTransform = quad.AddComponent<PrCore::ECS::TransformComponent>();
	quadTransform->SetPosition(PrCore::Math::vec3(20, 0, 20));
	quadTransform->SetRotation(PrCore::Math::quat(PrCore::Math::radians(PrCore::Math::vec3(90, 0, 0))));
	quadTransform->SetLocalScale(PrCore::Math::vec3(40, 40, 40));
	auto meshQuad = quad.AddComponent<PrCore::ECS::MeshRendererComponent>();
	meshQuad->material = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Material>("StressTest/floor.mat");
	meshQuad->mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Quad);

	// Walls
	/*auto logoEntity = scene->CreateEntity("Wall1");
	auto logoTransform = logoEntity.AddComponent<PrCore::ECS::TransformComponent>();
	auto logoMesh = logoEntity.AddComponent<PrCore::ECS::MeshRendererComponent>();
	logoTransform->SetPosition(PrCore::Math::vec3(20, 5.0f, -20));
	logoTransform->SetRotation(PrCore::Math::quat(PrCore::Math::radians(PrCore::Math::vec3(0, 0, 0))));
	logoTransform->SetLocalScale(PrCore::Math::vec3(80, 10, 1));
	logoMesh->material = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Material>("StressTest/brick.mat");
	logoMesh->mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Cube);

	logoEntity = scene->CreateEntity("Wall2");
	logoTransform = logoEntity.AddComponent<PrCore::ECS::TransformComponent>();
	logoMesh = logoEntity.AddComponent<PrCore::ECS::MeshRendererComponent>();
	logoTransform->SetPosition(PrCore::Math::vec3(20, 5.0f, 60));
	logoTransform->SetRotation(PrCore::Math::quat(PrCore::Math::radians(PrCore::Math::vec3(0, 0, 0))));
	logoTransform->SetLocalScale(PrCore::Math::vec3(80, 10, 1));
	logoMesh->material = PrCore::Resources::ResourceSystem::GetInstance().Get<PrRenderer::Resources::Material>("StressTest/brick.mat");
	logoMesh->mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Cube);

	logoEntity = scene->CreateEntity("Wall3");
	logoTransform = logoEntity.AddComponent<PrCore::ECS::TransformComponent>();
	logoMesh = logoEntity.AddComponent<PrCore::ECS::MeshRendererComponent>();
	logoTransform->SetPosition(PrCore::Math::vec3(-20, 5.0f, 20));
	logoTransform->SetRotation(PrCore::Math::quat(PrCore::Math::radians(PrCore::Math::vec3(0, 90, 0))));
	logoTransform->SetLocalScale(PrCore::Math::vec3(80, 10, 1));
	logoMesh->material = PrCore::Resources::ResourceSystem::GetInstance().Get<PrRenderer::Resources::Material>("StressTest/brick.mat");
	logoMesh->mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Cube);

	logoEntity = scene->CreateEntity("Wall4");
	logoTransform = logoEntity.AddComponent<PrCore::ECS::TransformComponent>();
	logoMesh = logoEntity.AddComponent<PrCore::ECS::MeshRendererComponent>();
	logoTransform->SetPosition(PrCore::Math::vec3(60, 5.0f, 20));
	logoTransform->SetRotation(PrCore::Math::quat(PrCore::Math::radians(PrCore::Math::vec3(0, 90, 0))));
	logoTransform->SetLocalScale(PrCore::Math::vec3(80, 10, 1));
	logoMesh->material = PrCore::Resources::ResourceSystem::GetInstance().Get<PrRenderer::Resources::Material>("StressTest/brick.mat");
	logoMesh->mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Cube);*/

	// Pan
	auto pan = scene->CreateEntity("Pan");
	auto transform = pan.AddComponent<PrCore::ECS::TransformComponent>();
	auto mesh = pan.AddComponent<PrCore::ECS::MeshRendererComponent>();

	auto panMeshShadow = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Mesh>("pan_lowPoly.obj");
	auto panMesh = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Mesh>("pan.obj");
	auto panMaterial = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Material>("StressTest/pan.mat");

	transform->SetPosition({ 35, 1.0f, 20 });
	transform->SetRotation(PrCore::Math::quat({ PrCore::Math::radians(0.0f), PrCore::Math::radians(0.f),PrCore::Math::radians(0.0f) }));
	transform->SetLocalScale(PrCore::Math::vec3(.2f));

	mesh->shadowMesh = panMeshShadow;
	mesh->mesh = panMesh;
	mesh->material = panMaterial;

	// Set Camera
	auto camera = scene->CreateEntity("Camera");
	camera.AddComponent<PrCore::ECS::CameraComponent>();
	auto cameraTransform = camera.AddComponent<PrCore::ECS::TransformComponent>();
	cameraTransform->SetPosition(PrCore::Math::vec3(40, 10, 20));
	cameraTransform->SetRotation(PrCore::Math::quat(PrCore::Math::radians(PrCore::Math::vec3(0, -250, 0))));

	PrCore::ECS::SceneManager::GetInstance().SaveSceneByName("RenderStressTest", "RenderStressTest.pearl");
	return;
	// Load Stress Test
	//PrCore::ECS::SceneManager::GetInstance().LoadScene("RenderStressTest.pearl");

	//auto shader = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Shader>("Shadows/PointShadowMappingNew.shader");

	//return;

	//auto scene1 = PrCore::ECS::SceneManager::GetInstance().LoadScene("SceneDeferred.pearl");

	////Add Sphere
	//auto sphereMaterial1 = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Material>("Deferred/PBR_IR_ValuesSphere_Transparent.mat");
	//sphereMaterial1->SetProperty("albedoValue", PrCore::Math::vec4(1.0f,0.0f, 0.0f, 0.6f));
	//sphereMaterial1->SetRenderType(PrRenderer::Resources::RenderType::Transparent);

	//auto sphereMesh1 = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Capsule);


	//auto transparentSphere = scene1->CreateEntity("Sphere Transparent");
	//auto spheretransform = transparentSphere.AddComponent<PrCore::ECS::TransformComponent>();
	//spheretransform->SetPosition(PrCore::Math::vec3(15.0f));
	//spheretransform->SetLocalScale(PrCore::Math::vec3(2, 2, 2));
	//auto meshRenderer1 = transparentSphere.AddComponent<PrCore::ECS::MeshRendererComponent>();
	//meshRenderer1->material = sphereMaterial1;
	//meshRenderer1->mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Quad);;


	////for (int i = 0; i < 20; i++)
	////{
	////	for (int j = 0; j < 20; j++)
	////	{
	////		auto sphere = scene1->CreateEntity("Sphere " + std::to_string(i) + "_" + std::to_string(j));
	////		auto spheretransform = sphere.AddComponent<PrCore::ECS::TransformComponent>();
	////		spheretransform->SetPosition(PrCore::Math::vec3(i * 5, j * 5, -10));
	////		spheretransform->SetLocalScale(PrCore::Math::vec3(2, 2, 2));
	////		auto meshRenderer1 = sphere.AddComponent<PrCore::ECS::MeshRendererComponent>();
	////		meshRenderer1->material = sphereMaterial1;
	////		meshRenderer1->mesh = sphereMesh1;
	////	}
	////}
	////
	//auto material2 = std::make_shared<PrRenderer::Resources::Material>(*PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Material>("Deferred/PBR_IR_ValuesSphere.mat"));
	//material2->SetProperty("albedoValue", PrCore::Math::vec4(0.0f, 1.0f, 0.5f, 0.0f));
	//material2->SetProperty("roughnessValue", 0.7f);
	//material2->SetProperty("metallicValue", 0.6f);
	//auto emissionTex = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Texture>("emission.png");
	//material2->SetTexture("emissionMap", emissionTex);
	//material2->SetProperty("emissionInt", 10.0f);
	//material2->SetProperty("emissionColor", PrCore::Math::vec3(1.0f, 0.0f, 0.0f));
	//material2->SetTexScale("albedoMap", PrCore::Math::vec2(1.0f, 1.0f));

	//material2->SetRenderType(PrRenderer::Resources::RenderType::Opaque);

	//for (int i = 0; i < 10; i++)
	//{
	//	for (int j = 0; j < 10; j++)
	//	{
	//		auto sphere = scene1->CreateEntity("Sphere " + std::to_string(i) + "_" + std::to_string(j));
	//		auto spheretransform = sphere.AddComponent<PrCore::ECS::TransformComponent>();
	//		spheretransform->SetPosition(PrCore::Math::vec3(i * 5, -5, j * 5));
	//		spheretransform->SetLocalScale(PrCore::Math::vec3(2, 2, 2));
	//		//spheretransform->SetRotation(PrCore::Math::vec3(0, 0, 3.14f / 2.f));
	//		auto meshRenderer1 = sphere.AddComponent<PrCore::ECS::MeshRendererComponent>();
	//		meshRenderer1->material = material2;
	//		meshRenderer1->mesh = sphereMesh1;
	//	}
	//}

	//{
	//	auto mainLight = scene1->CreateEntity("MainLight");
	//	auto lightComponent = mainLight.AddComponent<PrCore::ECS::LightComponent>();
	//	auto transformLight = mainLight.AddComponent<PrCore::ECS::TransformComponent>();
	//	transformLight->SetRotation(PrCore::Math::quat(PrCore::Math::radians(PrCore::Math::vec3(45.0f, -100.0f, 0.0f))));
	//	PrRenderer::Resources::LightPtr light = std::make_shared<PrRenderer::Resources::Light>();
	//	light->SetColor({ 0.8f,0.8f,.8f,1.0f });
	//	light->SetType(PrRenderer::Resources::LightType::Directional);
	//	lightComponent->m_light = light;
	//	lightComponent->mainDirectLight = true;
	//	lightComponent->m_shadowCast = true;
	//}

	//{
	//	auto mainLight1 = scene1->CreateEntity("LightDirectional");
	//	auto lightComponent1 = mainLight1.AddComponent<PrCore::ECS::LightComponent>();
	//	auto transformLight1 = mainLight1.AddComponent<PrCore::ECS::TransformComponent>();
	//	transformLight1->SetRotation(PrCore::Math::quat(PrCore::Math::radians(PrCore::Math::vec3(45.0f, 100.0f, 0.0f))));
	//	PrRenderer::Resources::LightPtr light1 = std::make_shared<PrRenderer::Resources::Light>();
	//	light1->SetColor({ 0.0f,0.8f,0.0f,1 });
	//	light1->SetType(PrRenderer::Resources::LightType::Directional);
	//	lightComponent1->m_light = light1;
	//	lightComponent1->mainDirectLight = false;
	//	lightComponent1->m_shadowCast = true;
	//}

	//auto gun = scene1->GetEntityByName("Gun");
	//gun.GetComponent<PrCore::ECS::TransformComponent>()->SetPosition({ 10,10,10 });

	////{
	////	auto mainLight2 = scene1->CreateEntity("MainLight2");
	////	auto lightComponent2 = mainLight2.AddComponent<PrCore::ECS::LightComponent>();
	////	auto transformLight2 = mainLight2.AddComponent<PrCore::ECS::TransformComponent>();
	////	transformLight2->SetRotation(PrCore::Math::quat(PrCore::Math::radians(PrCore::Math::vec3(45.0f, 180.0f, 0.0f))));
	////	PrRenderer::Resources::LightPtr light2 = std::make_shared<PrRenderer::Resources::Light>();
	////	light2->SetColor({ 1.0f,1.1f,1.0f,1 });
	////	light2->SetType(PrRenderer::Resources::LightType::Directional);
	////	lightComponent2->m_light = light2;
	////	lightComponent2->mainDirectLight = false;
	////}

	////{
	////	auto mainLight1 = scene1->CreateEntity("MainLight3");
	////	auto lightComponent1 = mainLight1.AddComponent<PrCore::ECS::LightComponent>();
	////	auto transformLight1 = mainLight1.AddComponent<PrCore::ECS::TransformComponent>();
	////	transformLight1->SetRotation(PrCore::Math::quat(PrCore::Math::radians(PrCore::Math::vec3(45.0f, 270.0f, 0.0f))));
	////	PrRenderer::Resources::LightPtr light1 = std::make_shared<PrRenderer::Resources::Light>();
	////	light1->SetColor({ 1.0f,1.1f,1.0f,1 });
	////	light1->SetType(PrRenderer::Resources::LightType::Directional);
	////	lightComponent1->m_light = light1;
	////	lightComponent1->mainDirectLight = false;
	////}

	//auto spotLight = scene1->CreateEntity("Light5");
	//auto spotLightComponent = spotLight.AddComponent<PrCore::ECS::LightComponent>();
	//auto transformspotLight = spotLight.AddComponent<PrCore::ECS::TransformComponent>();
	//transformspotLight->SetRotation(PrCore::Math::quat(PrCore::Math::radians(PrCore::Math::vec3( 90.0f, 0.0f, 0.0f ))));
	//transformspotLight->SetPosition({ 0,10,-5 });
	//PrRenderer::Resources::LightPtr lspotLight = std::make_shared<PrRenderer::Resources::Light>();
	//lspotLight->SetColor({ 20.0f,20.0f,0,1.0f });
	//lspotLight->SetType(PrRenderer::Resources::LightType::Spot);
	//lspotLight->SetInnerCone(20.0f);
	//lspotLight->SetRange(50.0f);
	//lspotLight->SetOutterCone(25.0f);
	//spotLightComponent->m_light = lspotLight;

	////mainLight.Destroy();
	////transformLight->SetLocalScale({ 0.5,0.5,0.5 });

	////auto meshRenderer1 = mainLight.AddComponent<PrCore::ECS::MeshRendererComponent>();
	////meshRenderer1->material = sphereMaterial1;
	////meshRenderer1->mesh = sphereMesh1;
	////transformLight->SetRotation(PrCore::Math::quat({ 0,90,0 }));

	////Quad to cast shadow
	//auto quad = scene1->CreateEntity("Quad");
	//auto quadTransform = quad.AddComponent<PrCore::ECS::TransformComponent>();
	//quadTransform->SetPosition(PrCore::Math::vec3(20, -6, 20));
	//quadTransform->SetRotation(PrCore::Math::quat(PrCore::Math::radians(PrCore::Math::vec3(90,0,0))));
	//quadTransform->SetLocalScale(PrCore::Math::vec3(40, 40, 40));
	//auto meshQuad = quad.AddComponent<PrCore::ECS::MeshRendererComponent>();
	//meshQuad->material = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Material>("PBR_IR_UniversalCross.mat");
	//meshQuad->material->SetTexScale("albedoMap", PrCore::Math::vec2(10.f, 10.f));
	//meshQuad->material->SetProperty("roughnessValue", 0.3f);
	////meshQuad->material = material2;
	//meshQuad->mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Quad);

	//auto unlightmaterial = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Material>("Deferred/PBR_IR_ValuesSphere_Unlit.mat");
	//auto lightObject = scene1->GetEntityByName("Light1");
	//auto lightComponent = lightObject.GetComponent<PrCore::ECS::LightComponent>();
	//auto lightRenderer = lightObject.AddComponent<PrCore::ECS::MeshRendererComponent>();
	//lightRenderer->material = std::make_shared<PrRenderer::Resources::Material>(*unlightmaterial);
	//auto color = static_cast<PrCore::Math::vec4>(lightComponent->m_light->GetColor());
	//lightRenderer->material->SetProperty("albedoValue", color);
	//lightRenderer->mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Sphere);;
	//lightRenderer->shadowCaster = false;
	//lightComponent->m_shadowCast = true;

	//lightObject = scene1->GetEntityByName("Light2");
	//lightComponent = lightObject.GetComponent<PrCore::ECS::LightComponent>();
	//lightRenderer = lightObject.AddComponent<PrCore::ECS::MeshRendererComponent>();
	//lightRenderer->material = std::make_shared<PrRenderer::Resources::Material>(*unlightmaterial);
	//color = static_cast<PrCore::Math::vec4>(lightComponent->m_light->GetColor());
	//lightRenderer->material->SetProperty("albedoValue", color);
	//lightRenderer->mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Sphere);;
	//lightRenderer->shadowCaster = false;
	//lightComponent->m_shadowCast = true;

	//lightObject = scene1->GetEntityByName("Light3");
	//lightComponent = lightObject.GetComponent<PrCore::ECS::LightComponent>();
	//lightRenderer = lightObject.AddComponent<PrCore::ECS::MeshRendererComponent>();
	//lightRenderer->material = std::make_shared<PrRenderer::Resources::Material>(*unlightmaterial);
	//color = static_cast<PrCore::Math::vec4>(lightComponent->m_light->GetColor());
	//lightRenderer->material->SetProperty("albedoValue", color);
	//lightRenderer->mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Sphere);;
	//lightRenderer->shadowCaster = false;
	//lightComponent->m_shadowCast = true;

	//lightObject = scene1->GetEntityByName("Light4");
	//lightComponent = lightObject.GetComponent<PrCore::ECS::LightComponent>();
	//lightRenderer = lightObject.AddComponent<PrCore::ECS::MeshRendererComponent>();
	//lightRenderer->material = std::make_shared<PrRenderer::Resources::Material>(*unlightmaterial);
	//color = static_cast<PrCore::Math::vec4>(lightComponent->m_light->GetColor());
	//lightRenderer->material->SetProperty("albedoValue", color);
	//lightRenderer->mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Sphere);;
	//lightRenderer->shadowCaster = false;
	//lightComponent->m_shadowCast = true;

	//lightObject = scene1->GetEntityByName("Light5");
	//lightComponent = lightObject.GetComponent<PrCore::ECS::LightComponent>();
	//lightRenderer = lightObject.AddComponent<PrCore::ECS::MeshRendererComponent>();
	//lightRenderer->material = std::make_shared<PrRenderer::Resources::Material>(*unlightmaterial);
	//color = static_cast<PrCore::Math::vec4>(lightComponent->m_light->GetColor());
	//lightRenderer->material->SetProperty("albedoValue", color);
	//lightRenderer->mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Cube);;
	//lightRenderer->shadowCaster = false;
	//lightComponent->m_shadowCast = true;

	//scene1->GetEntityByName("Light1").Destroy();
	return;

	//auto scene = PrCore::ECS::SceneManager::GetInstance().CreateScene("TESTSCENE");
	//scene->RegisterSystem<PrCore::ECS::HierarchyTransform>();
	//scene->RegisterSystem<PrCore::ECS::MeshRendererSystem>();
	//scene->RegisterSystem<PrCore::ECS::TestSystem>();
	//
	////Add Camera
	//auto camera = scene->CreateEntity("Camera");
	////camera.AddComponent<PrCore::ECS::CameraComponent>();
	//camera.AddComponent<PrCore::ECS::TransformComponent>();
	//
	////Add Lights
	//auto light = scene->CreateEntity("Light1");
	//auto lightComponent = light.AddComponent<PrCore::ECS::LightComponent>();
	//lightComponent->SetType(PrCore::ECS::LightType::Point);
	//lightComponent->SetColor({ 300, 300, 300 });
	//auto transformComponent = light.AddComponent<PrCore::ECS::TransformComponent>();
	//transformComponent->SetPosition({ 0, 2, 10 });
	//
	//light = scene->CreateEntity("Light2");
	//lightComponent = light.AddComponent<PrCore::ECS::LightComponent>();
	//lightComponent->SetType(PrCore::ECS::LightType::Point);
	//lightComponent->SetColor({ 300, 300, 300 });
	//transformComponent = light.AddComponent<PrCore::ECS::TransformComponent>();
	//transformComponent->SetPosition({ 0, 2, 5 });
	//
	//light = scene->CreateEntity("Light3");
	//lightComponent = light.AddComponent<PrCore::ECS::LightComponent>();
	//lightComponent->SetType(PrCore::ECS::LightType::Point);
	//lightComponent->SetColor({ 300, 300, 0 });
	//transformComponent = light.AddComponent<PrCore::ECS::TransformComponent>();
	//transformComponent->SetPosition({ 0, -2, 10 });
	//
	//light = scene->CreateEntity("Light4");
	//lightComponent = light.AddComponent<PrCore::ECS::LightComponent>();
	//lightComponent->SetType(PrCore::ECS::LightType::Point);
	//lightComponent->SetColor({ 300, 0, 300 });
	//transformComponent = light.AddComponent<PrCore::ECS::TransformComponent>();
	//transformComponent->SetPosition({ 0, -2, 5 });
	//
	////Add Sphere
	//auto sphereMaterial = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Material>("PBR_IR_Values.mat");
	//sphereMaterial->SetProperty("roughnessValue", 0.6f);
	//sphereMaterial->SetProperty("metallicValue", 0.99f);
	//sphereMaterial->SetProperty("albedoValue", PrCore::Math::vec3(1.0f, 0.65f, 0.0f));
	//sphereMaterial->SetProperty("aoValue", 1.0f);
	//
	//auto sphereMesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Sphere);
	//
	//
	//
	//auto sphere = scene->CreateEntity("Sphere");
	//auto spheretransform = sphere.AddComponent<PrCore::ECS::TransformComponent>();
	//spheretransform->SetPosition(PrCore::Math::vec3(0, 5, -10));
	//auto meshRenderer = sphere.AddComponent<PrCore::ECS::MeshRendererComponent>();
	//
	//meshRenderer->material = sphereMaterial;
	//meshRenderer->mesh = sphereMesh;
	//
	////Add Gun
	//auto gun = scene->CreateEntity("Gun");
	//auto gunTransform = gun.AddComponent<PrCore::ECS::TransformComponent>();
	//auto gunMeshRenderer = gun.AddComponent<PrCore::ECS::MeshRendererComponent>();
	//
	//auto gunMesh = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Mesh>("yo.obj");
	//auto gunMaterial = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Material>("PBR_IR_UniversalGun.mat");
	//gunMaterial->SetProperty("normalMapping", true);
	//gunMaterial->SetProperty("aoValue", 1.0f);
	//
	//gunTransform->SetPosition({ 0, 2, 10 });
	//gunTransform->SetRotation(PrCore::Math::quat({ PrCore::Math::radians(0.0f), PrCore::Math::radians(0.f),PrCore::Math::radians(0.0f) }));
	//gunTransform->SetLocalScale(PrCore::Math::vec3(0.05f));
	//
	//gunMeshRenderer->mesh = gunMesh;
	//gunMeshRenderer->material = gunMaterial;
	//
	////Add Gun
	//auto pan = scene->CreateEntity("Pan");
	//auto panTransform = pan.AddComponent<PrCore::ECS::TransformComponent>();
	//auto panMeshRenderer = pan.AddComponent<PrCore::ECS::MeshRendererComponent>();
	//
	//auto panMesh = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Mesh>("pan.obj");
	//auto panMaterial = PrCore::Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Material>("PBR_IR_UniversalPan.mat");
	//panMaterial->SetProperty("normalMapping", true);
	//panMaterial->SetProperty("aoValue", 1.0f);
	//
	//panTransform->SetPosition({ 0, 10, 10 });
	//panTransform->SetRotation(PrCore::Math::quat({ PrCore::Math::radians(0.0f), PrCore::Math::radians(0.f),PrCore::Math::radians(0.0f) }));
	//panTransform->SetLocalScale(PrCore::Math::vec3(0.5f));
	//
	//panMeshRenderer->mesh = panMesh;
	//panMeshRenderer->material = panMaterial;
	//
	//PrCore::Utils::JSON::json json;
	//scene->OnSerialize(json);
	//auto dump = json.dump(4);
}

TestFeatures::~TestFeatures()
{}

void TestFeatures::Update(float p_deltaTime)
{
}
