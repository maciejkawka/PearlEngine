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
	PrRenderer::Core::Renderer3D::GetInstance().Begin();

	//Exit
	if (PrCore::Input::InputManager::IsKeyPressed(PrCore::Input::PrKey::ESCAPE))
		m_shouldClose = true;
}

void Editor::OnFrame(float p_deltaTime)
{
	m_basicCamera->Update(p_deltaTime);

	//Camera Settings
	if (PrCore::Input::InputManager::IsKeyPressed(PrCore::Input::PrKey::O))
		PrRenderer::Core::Camera::GetMainCamera()->SetType(PrRenderer::Core::CameraType::Ortographic);

	if (PrCore::Input::InputManager::IsKeyPressed(PrCore::Input::PrKey::P))
		PrRenderer::Core::Camera::GetMainCamera()->SetType(PrRenderer::Core::CameraType::Perspective);

	//Show Mouse Pos
	if (PrCore::Input::InputManager::IsKeyHold(PrCore::Input::PrKey::LEFT_CONTROL))
	{
		auto mousePos = PrCore::Input::InputManager::GetMousePosition();
		auto xPos = mousePos.x;
		auto yPos = mousePos.y;

		PRLOG_INFO("Mouse Pos x: {0}, y: {1}", xPos, yPos);
	}

	//Show FPS
	if (PrCore::Input::InputManager::IsKeyHold(PrCore::Input::PrKey::F1))
		PRLOG_INFO("{0}", (int)(1 / p_deltaTime));

	m_testFeatures->Update(p_deltaTime);

	//Scene Update
	auto scenes = PrCore::ECS::SceneManager::GetInstance().GetAllScenes();

	for(auto scene : scenes)
	{
		scene->Update(p_deltaTime);

		//Phisics Tick
		scene->FixUpdate(p_deltaTime);
		//

		scene->LateUpdate(p_deltaTime);

		scene->UpdateHierrarchicalEntities(p_deltaTime);
		scene->CleanDestroyedEntities();

		scene->RenderUpdate(p_deltaTime);
	}
}

void Editor::PostFrame()
{
	PrRenderer::Core::Renderer3D::GetInstance().Flush();
	m_appContext->m_window->SwapBuffers();
	m_appContext->m_input->ResetFlags();
	
	EventManager::GetInstance().Update();
}


