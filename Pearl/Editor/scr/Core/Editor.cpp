#include"Editor/Core/Editor.h"

#include "Core/ECS/Scene.h"
#include"Editor/Core/EditorContext.h"
#include"Engine/Core/Utils/Logger.h"
#include"Engine/Core/Events/EventManager.h"
#include"Engine/Core/ECS/SceneManager.h"

#include"Renderer/Resources/ShaderManager.h"
#include"Renderer/Resources/TextureManager.h"
#include"Renderer/Resources/MaterialManager.h"
#include"Renderer/Resources/Shader.h"
#include"Renderer/Resources/Material.h"
#include"Renderer/Core/DeferredRendererFrontend.h"

using namespace PrEditor::Core;
using namespace PrCore::Events;

Editor::Editor()
{
	m_appContext = new EditorContext();
	m_testFeatures = new Components::TestFeatures();
	m_basicCamera = new Components::BasicCamera(PrRenderer::Core::CameraType::Perspective);
	m_basicCamera->GetCamera()->SetSize(5.0f);
}

Editor::~Editor()
{
	delete m_basicCamera;
	delete m_appContext;
	delete m_testFeatures;
}

void Editor::PreFrame()
{
	m_appContext->m_window->PollEvents();
	PrRenderer::Core::DefferedRendererFrontend::GetInstance().PrepareFrame();

	//Exit
	if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(PrCore::Input::PrKey::ESCAPE))
		m_shouldClose = true;

	PrRenderer::Core::DefferedRendererFrontend::GetInstance().AddCamera(m_basicCamera->GetCamera());
}

void Editor::OnFrame(float p_deltaTime)
{
	m_basicCamera->Update(p_deltaTime);

	//Camera Settings
	if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(PrCore::Input::PrKey::O))
		PrRenderer::Core::Camera::GetMainCamera()->SetType(PrRenderer::Core::CameraType::Ortographic);

	if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(PrCore::Input::PrKey::P))
		PrRenderer::Core::Camera::GetMainCamera()->SetType(PrRenderer::Core::CameraType::Perspective);

	//Show Mouse Pos
	if (PrCore::Input::InputManager::GetInstance().IsKeyHold(PrCore::Input::PrKey::LEFT_CONTROL))
	{
		auto mousePos = PrCore::Input::InputManager::GetInstance().GetMousePosition();
		auto xPos = mousePos.x;
		auto yPos = mousePos.y;

		PRLOG_INFO("Mouse Pos x: {0}, y: {1}", xPos, yPos);
	}

	//Show FPS
	if (PrCore::Input::InputManager::GetInstance().IsKeyHold(PrCore::Input::PrKey::F1))
		PRLOG_INFO("{0}", (int)(1 / p_deltaTime));

	m_testFeatures->Update(p_deltaTime);

	//Scene Update
	auto scenes = PrCore::ECS::SceneManager::GetInstance().GetAllScenes();

	for(auto scene : scenes)
	{
		scene->OnEnable();

		scene->Update(p_deltaTime);

		//Phisics Tick
		scene->FixUpdate(p_deltaTime);
		//

		scene->LateUpdate(p_deltaTime);

		scene->UpdateHierrarchicalEntities(p_deltaTime);
		scene->CleanDestroyedEntities();

		scene->RenderUpdate(p_deltaTime);

		scene->OnDisable();
	}

	auto testInfo = PrRenderer::Core::DefferedRendererFrontend::GetInstance().GetPreviousFrameInfo();
	
	if (PrCore::Input::InputManager::GetInstance().IsKeyHold(PrCore::Input::PrKey::F3))
		for (auto event : testInfo.timeEvents)
			PRLOG_INFO("Event {0}, Time: {1}", event.first, event.second);

	PrRenderer::Core::DefferedRendererFrontend::GetInstance().BuildFrame();
	m_appContext->m_rendererBackend->PreRender();
}

void Editor::PostFrame()
{
	m_appContext->m_rendererBackend->Render();
	m_appContext->m_rendererBackend->PostRender();

	m_appContext->m_window->SwapBuffers();
	PrCore::Input::InputManager::GetInstance().ResetFlags();
	
	EventManager::GetInstance().Update();
}


