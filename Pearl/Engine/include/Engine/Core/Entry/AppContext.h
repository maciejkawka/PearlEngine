#pragma once
#include"Core/Windowing/GLWindow.h"
#include"Core/Input/InputManager.h"

#include"Renderer/Core/Context.h"

#include<memory>

namespace PrCore::Entry {
	
	class AppContext {
	public:
		AppContext();
		virtual ~AppContext();

		PrCore::Windowing::GLWindow* m_window;
		PrRenderer::Core::Context* m_rendererContext;
	};
}