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
		m_testFeatures = new Components::TestFeatures();
		m_basicCamera = new Components::BasicCamera(PrRenderer::CameraType::Perspective);
		m_basicCamera->GetCamera()->SetSize(5.0f);
		PrSystems::Get<PrRenderer::IRenderFrontend>()->SetCamera(m_basicCamera->GetCamera());

		PrCore::EventListener windowCloseListener;
		windowCloseListener.Connect<&EditorContext::OnWindowClose>(this);
		PrSystems::Get<PrCore::EventManager>()->AddListener(windowCloseListener, PrCore::WindowCloseEvent::s_type);

		return true;
	}

	void EditorContext::OnTerminate()
	{
		PRLOG_INFO("Terminating EditorContext");

		delete m_basicCamera;
		delete m_testFeatures;

		PrSystems::Get<PrCore::ResourceSystem>()->UnregisterDatabase<Assets::ModelResource>();
	}

	bool EditorContext::OnUpdate(float p_dt)
	{
		m_basicCamera->Update(p_dt);
		m_testFeatures->Update(p_dt);

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
