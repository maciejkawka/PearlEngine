#pragma once
#include"Core/Windowing/GLWindow.h"
#include"Core/Input/InputManager.h"

#include"Renderer/Core/Context.h"
#include"Renderer/Core/Renderer3D.h"

#include<memory>

namespace PrCore::Entry {
	
	//Place to hold window and all application context variables
	class AppContext {
	public:
		AppContext();

		virtual ~AppContext();

		PrCore::Windowing::GLWindow* m_window;
		PrRenderer::Core::Context* m_rendererContext;
		//PrRenderer::Core::Renderer* m_renderer;
		PrRenderer::Core::Renderer3D* m_renderer3D;
		PrCore::Input::InputManager* m_input;
	};
}