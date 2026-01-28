#include "Editor/Core/Editor.h"

#include "Core/ECS/Scene.h"
#include "Editor/Core/EditorContext.h"
#include "Engine/Core/Utils/Logger.h"
#include "Engine/Core/Events/EventManager.h"
#include "Engine/Core/ECS/SceneManager.h"

#include "Renderer/Resources/Shader.h"
#include "Renderer/Resources/Material.h"
#include "Renderer/Core/IRenderFrontend.h"
#include "Physics/Core/PhysicsSystem.h"
#include "Editor/Assets/Exporter/SceneExporter.h"

using namespace PrEditor::Core;
using namespace PrCore;

Editor::Editor()
{
	m_appContext = new EditorContext();
	m_testFeatures = new Components::TestFeatures();
	m_basicCamera = new Components::BasicCamera(PrRenderer::CameraType::Perspective);
	m_basicCamera->GetCamera()->SetSize(5.0f);

	PrSystems::Get<PrRenderer::IRenderFrontend>()->SetCamera(m_basicCamera->GetCamera());
}

Editor::~Editor()
{
	delete m_basicCamera;
	delete m_testFeatures;
	delete m_appContext;
}

void Editor::PreFrame()
{
	m_appContext->m_window->PollEvents();
	PrSystems::Get<PrRenderer::IRenderFrontend>()->PrepareFrame();

	//Exit
	if (PrSystems::Get<PrCore::InputManager>()->IsKeyPressed(PrCore::PrKey::ESCAPE))
		m_shouldClose = true;
}

void Editor::OnFrame(float p_deltaTime)
{
	m_basicCamera->Update(p_deltaTime);

	//Camera Settings
	if (PrSystems::Get<PrCore::InputManager>()->IsKeyPressed(PrCore::PrKey::O))
		PrRenderer::Camera::GetMainCamera()->SetType(PrRenderer::CameraType::Ortographic);

	if (PrSystems::Get<PrCore::InputManager>()->IsKeyPressed(PrCore::PrKey::P))
		PrRenderer::Camera::GetMainCamera()->SetType(PrRenderer::CameraType::Perspective);

	//Show Mouse Pos
	if (PrSystems::Get<PrCore::InputManager>()->IsKeyHold(PrCore::PrKey::LEFT_CONTROL))
	{
		auto mousePos = PrSystems::Get<PrCore::InputManager>()->GetMousePosition();
		auto xPos = mousePos.x;
		auto yPos = mousePos.y;

		PRLOG_INFO("Mouse Pos x: {0}, y: {1}", xPos, yPos);
	}

	//Show FPS
	if (PrSystems::Get<PrCore::InputManager>()->IsKeyHold(PrCore::PrKey::F1))
		PRLOG_INFO("{0}", (int)(1 / p_deltaTime));

	m_testFeatures->Update(p_deltaTime);

	//Scene Update
	auto scenes = PrCore::ECS::SceneManager::GetInstance().GetAllScenes();

	for (auto scene : scenes)
	{
		scene->OnEnable();

		scene->Update(p_deltaTime);

		//Physics Tick
		m_physicsStepAccumulator += p_deltaTime;
		int i = 0;
		while (m_physicsStepAccumulator >= m_physicsFixStep)
		{
			scene->PhysicsUpdate(m_physicsFixStep);
			scene->FixUpdate(m_physicsFixStep);
			m_physicsStepAccumulator -= m_physicsFixStep;
		}

		scene->LateUpdate(p_deltaTime);

		scene->UpdateHierrarchicalEntities(p_deltaTime);

		scene->RenderUpdate(p_deltaTime);

		scene->OnDisable();

		scene->CleanDestroyedEntities();
		scene->PhysicsCleanup(p_deltaTime);
	}

	auto pRenderer = PrSystems::Get<PrRenderer::IRenderFrontend>();
	auto testInfo = pRenderer->GetPreviousFrameInfo();
	
	if (PrSystems::Get<PrCore::InputManager>()->IsKeyHold(PrCore::PrKey::F3))
		for (auto event : testInfo.timeEvents)
			PRLOG_INFO("Event {0}, Time: {1}", event.first, event.second);

	if (PrSystems::Get<PrCore::InputManager>()->IsKeyHold(PrCore::PrKey::F4))
		PRLOG_INFO("Culled objects {0}", testInfo.culledObjects);

	if (PrSystems::Get<PrCore::InputManager>()->IsKeyHold(PrCore::PrKey::F5))
		PRLOG_INFO("Draw triangles {0}", testInfo.drawTriangles);

	if (PrSystems::Get<PrCore::InputManager>()->IsKeyHold(PrCore::PrKey::F6))
		PRLOG_INFO("Draw objects {0}", testInfo.drawObjects	);

	pRenderer->BuildFrame();
	pRenderer->GetRendererBackend()->PreparePipeline();
}

void Editor::PostFrame()
{
	PrSystems::Get<PrRenderer::IRenderFrontend>()->GetRendererBackend()->Render();
	PrSystems::Get<PrRenderer::IRenderFrontend>()->GetRendererBackend()->PostRender();

	m_appContext->m_window->SwapBuffers();
	PrSystems::Get<PrCore::InputManager>()->ResetFlags();

	PrSystems::Get<EventManager>()->Update();

	if (PrSystems::Get<PrCore::InputManager>()->IsKeyHold(PrCore::PrKey::F8))
	{
		Assets::SceneExporter exporter;
		exporter.SaveMemoryResourcesToFile("OceanNew/");
		auto scene = PrCore::ECS::SceneManager::GetInstance().GetActiveScene();
		PrCore::ECS::SceneManager::GetInstance().SaveSceneByReference(scene, "scene/scene_export.pearl");
	}
}


