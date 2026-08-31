#include "Editor/Core/EditorContext.h"

#include "Editor/Assets/Model/ModelResource.h"
#include "Editor/Assets/Model/ModelResourceLoader.h"

#include "Engine/Core/Events/WindowEvents.h"
#include "Engine/Core/Resources/ResourceDatabase.h"
#include "Engine/Core/Utils/Logger.h"
#include "Engine/Renderer/Core/IRenderFrontend.h"

namespace PrEditor {

	bool EditorContext::OnInitalize(const PrCore::EngineCoreParams& p_params)
	{
		PRLOG_INFO("Initializing EditorContext");

		// Register Editor Resources
		auto modelDatabase = std::make_unique<PrCore::ResourceDatabase>();
		modelDatabase->RegisterLoader(".glb", std::make_unique<Assets::ModelResourceLoader>());
		modelDatabase->RegisterLoader(".fbx", std::make_unique<Assets::ModelResourceLoader>());
		modelDatabase->RegisterLoader(".gltf", std::make_unique<Assets::ModelResourceLoader>());
		PrSystems::Get<PrCore::ResourceSystem>()->RegisterDatabase<Assets::ModelResource>(std::move(modelDatabase));

		// Test Components
		m_basicCamera = new Components::BasicCamera(PrRenderer::CameraType::Perspective);
		m_basicCamera->GetCamera()->SetSize(5.0f);
		PrSystems::Get<PrRenderer::IRenderFrontend>()->SetCamera(m_basicCamera->GetCamera());

		PrCore::EventListener windowCloseListener;
		windowCloseListener.Connect<&EditorContext::OnWindowClose>(this);
		PrSystems::Get<PrCore::EventManager>()->AddListener(windowCloseListener, PrCore::WindowCloseEvent::s_type);


		// Load Cheess Project
		auto scene101 = PrSystems::Get<PrCore::SceneManager>()->CreateScene("ChessGame");

		auto modeHandl = PrSystems::Get<PrCore::ResourceSystem>()->Load<Assets::ModelResource>("Chess/ChessCopy.glb");
		modeHandl->AddEntitesToScene(scene101);

		auto root = scene101->GetEntityByName("chesscopy").GetComponent<PrCore::TransformComponent>();
		root->SetPosition(m_basicCamera->GetPosition() + PrCore::Math::vec3{0.0f, 0.0f, 0.0f});
		root->SetLocalScale(PrCore::Math::vec3{ 1.0f });

		scene101->RegisterSystem<PrCore::HierarchyTransform>();
		scene101->RegisterSystem<PrCore::MeshRendererSystem>();
		scene101->RegisterSystem<PrCore::PhysicsUpdateSystem>();


		//auto light = std::make_shared<PrRenderer::Light>();
		//light->SetType(PrRenderer::LightType::Directional);
		//light->SetColor({ 2.5f,2.5f,2.5f,1.0f });

		//auto lightEntity = scene101->CreateEntity("MainLight");
		//auto lightComponent = lightEntity.AddComponent<PrCore::LightComponent>();
		//lightComponent->m_light = light;
		//lightComponent->mainDirectLight = true;

		//auto loightTransform = lightEntity.AddComponent<PrCore::TransformComponent>();
		//loightTransform->SetRotation({ -0.9f, 0.35f, 0.0f, 0.0f });
		m_basicCamera->GetCamera()->SetNear(0.001f);
		m_basicCamera->GetCamera()->SetPosition({ 7.0f, 1.0f, -2.0f });
		m_basicCamera->GetCamera()->SetRotation({ -0.46f, 2.17f, 0.0f });



		PrSystems::Get<PrRenderer::IRenderFrontend>()->SetCubemap(PrSystems::Get<PrCore::ResourceSystem>()->Load<PrRenderer::Material>("outdoor/suburban.mat").GetData());
		return true;
	}

	void EditorContext::OnTerminate()
	{
		PRLOG_INFO("Terminating EditorContext");

		delete m_basicCamera;

		PrSystems::Get<PrCore::ResourceSystem>()->UnregisterDatabase<Assets::ModelResource>();
	}

	bool EditorContext::OnUpdate(float p_dt)
	{
		m_basicCamera->Update(p_dt);

		//Show FPS
		if (PrSystems::Get<PrCore::InputManager>()->IsKeyHold(PrCore::PrKey::F1))
			PRLOG_INFO("{0}", (int)(1 / p_dt));

		//Exit
		if (PrSystems::Get<PrCore::InputManager>()->IsKeyPressed(PrCore::PrKey::ESCAPE))
			return false;

		return !m_shouldClose;
	}

	bool EditorContext::ShouldClose()
	{
		return m_shouldClose;
	}

	void EditorContext::OnWindowClose(PrCore::EventPtr p_event)
	{
		m_shouldClose = true;
	}
}
