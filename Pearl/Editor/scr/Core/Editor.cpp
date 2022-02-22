#include"Editor/Core/Editor.h"
#include"Editor/Core/EditorContext.h"
#include"Engine/Core/Utils/Logger.h"
#include"Engine/Core/Events/EventManager.h"

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
	m_basicCamera = new Components::BasicCamera(PrRenderer::Core::CameraType::Perspective);
	m_basicCamera->GetCamera()->SetSize(5.0f);
	m_scale = PrCore::Math::vec2(1, 1);
}

Editor::~Editor()
{
	delete m_basicCamera;
	delete m_appContext;
}

void Editor::PreFrame()
{
	m_appContext->m_window->PollEvents();
}

void Editor::OnFrame(float p_deltaTime)
{
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
		PRLOG_INFO("{0}", (int)(1/p_deltaTime));

	//Change Shader
	if (PrCore::Input::InputManager::IsKeyPressed(PrCore::Input::PrKey::K))
	{
		PrRenderer::Resources::ShaderPtr shader = std::static_pointer_cast<PrRenderer::Resources::Shader>(PrRenderer::Resources::ShaderManager::GetInstance().GetResource("CameraShader.shader"));
		shader->Bind();
	}

	//Change Shader
	if (PrCore::Input::InputManager::IsKeyPressed(PrCore::Input::PrKey::L))
	{
		PrRenderer::Resources::ShaderPtr shader = std::static_pointer_cast<PrRenderer::Resources::Shader>(PrRenderer::Resources::ShaderManager::GetInstance().GetResource("BasicShader.shader"));
		shader->Bind();
	}

	//Camera Settings
	if (PrCore::Input::InputManager::IsKeyPressed(PrCore::Input::PrKey::O))
		PrRenderer::Core::Camera::GetMainCamera()->SetType(PrRenderer::Core::CameraType::Ortographic);

	if (PrCore::Input::InputManager::IsKeyPressed(PrCore::Input::PrKey::P))
		PrRenderer::Core::Camera::GetMainCamera()->SetType(PrRenderer::Core::CameraType::Perspective);

	if (PrCore::Input::InputManager::IsKeyHold(PrCore::Input::PrKey::H))
	{
		auto material = std::static_pointer_cast<PrRenderer::Resources::Material>(PrRenderer::Resources::MaterialManager::GetInstance().GetResource("standardMaterial.mat"));
		m_scale -= 0.05f;
		if (m_scale.x <= 0.05f)
			m_scale = PrCore::Math::vec2(0.05f, 0.05f);

		material->SetTexScale("mainTex", m_scale);
	
	}
	if (PrCore::Input::InputManager::IsKeyHold(PrCore::Input::PrKey::G))
	{
		auto material = std::static_pointer_cast<PrRenderer::Resources::Material>(PrRenderer::Resources::MaterialManager::GetInstance().GetResource("standardMaterial.mat"));
		m_scale += 0.05f;
		material->SetTexScale("mainTex", m_scale);

	}

	if (PrCore::Input::InputManager::IsKeyPressed(PrCore::Input::PrKey::R))
	{
		auto material = std::static_pointer_cast<PrRenderer::Resources::Material>(PrRenderer::Resources::MaterialManager::GetInstance().GetResource("standardMaterial.mat"));
		std::string matrix = "MVP";
		auto& returnValue = material->GetProperty<PrCore::Math::mat4>(matrix);
	}

	//Exit
	if (PrCore::Input::InputManager::IsKeyPressed(PrCore::Input::PrKey::ESCAPE))
		m_shouldClose = true;

	m_basicCamera->Update(p_deltaTime);

	auto material = std::static_pointer_cast<PrRenderer::Resources::Material>(PrRenderer::Resources::MaterialManager::GetInstance().GetResource("standardMaterial.mat"));
	auto color = material->GetColor();

	if (PrCore::Input::InputManager::IsKeyPressed(PrCore::Input::PrKey::C))
		material->SetColor(PrRenderer::Core::Color::Red);
	if (PrCore::Input::InputManager::IsKeyPressed(PrCore::Input::PrKey::V))
		material->SetColor(PrRenderer::Core::Color::Green);
	if (PrCore::Input::InputManager::IsKeyPressed(PrCore::Input::PrKey::B))
		material->SetColor(PrRenderer::Core::Color::White);
}

void Editor::PostFrame()
{
	m_appContext->m_renderer->Draw();
	m_appContext->m_window->SwapBuffers();
	m_appContext->m_input->ResetFlags();
	
	EventManager::GetInstance().Update();
}


