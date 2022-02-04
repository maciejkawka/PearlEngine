#include"Editor/Core/Editor.h"
#include"Editor/Core/EditorContext.h"
#include"Engine/Core/Utils/Logger.h"
#include"Engine/Core/Events/EventManager.h"

#include"Renderer/Resources/ShaderManager.h"
#include"Renderer/Resources/TextureManager.h"
#include"Renderer/Resources/Shader.h"

using namespace PrEditor::Core;
using namespace PrCore::Events;

Editor::Editor()
{
	m_appContext = new EditorContext();
}

Editor::~Editor()
{
	delete m_appContext;
}
void Editor::PreFrame()
{
	m_appContext->m_window->PollEvents();
}

void Editor::OnFrame(float p_deltaTime)
{
	if (PrCore::Input::InputManager::IsKeyHold(PrCore::Input::PrKey::LEFT_CONTROL))
	{
		auto mousePos = PrCore::Input::InputManager::GetMousePosition();
		auto xPos = mousePos.x;
		auto yPos = mousePos.y;

		PRLOG_INFO("Mouse Pos x: {0}, y: {1}", xPos, yPos);
	}

	if (PrCore::Input::InputManager::IsAnyKeyPressed())
		PRLOG_INFO("ANY PRESSED");

	//Show FPS
	if (PrCore::Input::InputManager::IsKeyHold(PrCore::Input::PrKey::LEFT_SHIFT))
		PRLOG_INFO("{0}", 1/p_deltaTime);


	if (PrCore::Input::InputManager::IsKeyPressed(PrCore::Input::PrKey::K))
	{
		PrRenderer::Resources::ShaderPtr shader = std::static_pointer_cast<PrRenderer::Resources::Shader>(PrRenderer::Resources::ShaderManager::GetInstance().GetResource("TextureShader.shader"));
		shader->Bind();
	}

	if (PrCore::Input::InputManager::IsKeyPressed(PrCore::Input::PrKey::L))
	{
		PrRenderer::Resources::ShaderPtr shader = std::static_pointer_cast<PrRenderer::Resources::Shader>(PrRenderer::Resources::ShaderManager::GetInstance().GetResource("BasicShader.shader"));
		shader->Bind();
	}

	if (PrCore::Input::InputManager::IsKeyPressed(PrCore::Input::PrKey::ESCAPE))
		m_shouldClose = true;	
	
	if (PrCore::Input::InputManager::IsKeyPressed(PrCore::Input::PrKey::SPACE))
		PRLOG_INFO("{0} bytes",PrRenderer::Resources::TextureManager::GetInstance().GetMemoryUsage());
}

void Editor::PostFrame()
{
	m_appContext->m_renderer->Draw();
	m_appContext->m_window->SwapBuffers();
	m_appContext->m_input->ResetFlags();
	
	EventManager::GetInstance().Update();
}


