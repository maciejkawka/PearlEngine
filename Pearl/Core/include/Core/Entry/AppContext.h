#pragma once
#include"Core/Windowing/GLWindow.h"
#include"Core/Input/InputManager.h"
#include<memory>

namespace PrCore::Entry {
	
	//Place to hold window and all application context variables
	class AppContext {
	public:
		AppContext();

		virtual ~AppContext();

		std::unique_ptr<PrCore::Windowing::GLWindow> m_window;
		std::unique_ptr<PrCore::Input::InputManager> m_input;
	};
}