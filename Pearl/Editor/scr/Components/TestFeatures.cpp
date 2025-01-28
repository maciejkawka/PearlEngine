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
#include"Core/Utils/JSONParser.h"
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
#include "Renderer/Resources/Light.h"

#include "Core/Threading/JobSystem.h"
#include "Core/Threading/ThreadSystem.h"

//
using namespace PrEditor::Components;

TestFeatures::TestFeatures()
{
	using namespace PrCore::Resources;
	using namespace PrRenderer::Resources;

	// Load Stress Test
	auto scene10 = PrCore::ECS::SceneManager::GetInstance().CreateScene("scene/render_stress_test.pearl");
	scene10->RegisterSystem<PrCore::ECS::HierarchyTransform>();
	scene10->RegisterSystem<PrCore::ECS::MeshRendererSystem>();
	scene10->RegisterSystem<PrCore::ECS::RenderStressTest>();

	auto modeHandl = PrCore::Resources::ResourceSystem::GetInstance().Load<Assets::ModelResource>("Model/ocean.glb");
	modeHandl->AddEntitesToScene(scene10);

	auto root = scene10->GetEntityByName("ocean").GetComponent<PrCore::ECS::TransformComponent>();
	root->SetPosition(PrCore::Math::vec3{ 0,5.0f,0.0f });
}

TestFeatures::~TestFeatures()
{}

void TestFeatures::Update(float p_deltaTime)
{
}
