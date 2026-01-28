#pragma once

#include "Core/Windowing/Window.h"
#include "Core/Input/InputManager.h"

#include "Renderer/Core/Context.h"

#include <memory>

namespace PrCore::Entry {
	
	class AppContext {
	public:
		AppContext();
		virtual ~AppContext();

		PrCore::IWindow* m_window;
	};
}