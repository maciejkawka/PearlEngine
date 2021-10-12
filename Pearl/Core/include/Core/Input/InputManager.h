#pragma once
#include"PrKey.h"
#include"PrMouseButton.h"
#include"PrKeyState.h"
#include"Core/Events/Event.h"
#include<unordered_map>

namespace PrCore::Input {

	class InputManager {

		typedef std::unordered_map<PrCore::Input::PrKey, PrCore::Input::PrInputState>			KeyStateMap;
		typedef std::unordered_map<PrCore::Input::PrKey, bool>									KeyHoldMap;
		typedef std::unordered_map<PrCore::Input::PrMouseButton, PrCore::Input::PrInputState>	ButtonStateMap;
		typedef std::unordered_map<PrCore::Input::PrMouseButton, bool>							ButtonHoldMap;

	public:
		InputManager();

		~InputManager();

		static bool IsKeyPressed(PrCore::Input::PrKey p_key);
		static bool IsKeyReleased(PrCore::Input::PrKey p_key);
		static bool IsKeyHold(PrCore::Input::PrKey p_key);

		static bool IsButtonPressed(PrCore::Input::PrMouseButton p_key);
		static bool IsButtonReleased(PrCore::Input::PrMouseButton p_key);
		static bool IsButtonHold(PrCore::Input::PrMouseButton p_key);

		static bool IsAnyKeyPressed();
		static bool IsAnyKeyHold();

		static double GetMouseScroll();
		static double GetMouseX();
		static double GetMouseY();
		//static Vector2 GetMousePosition();

		void ResetFlags();

	private:
		void OnKeyPressed(PrCore::Events::EventPtr p_event);
		void OnKeyReleased(PrCore::Events::EventPtr p_event);

		void OnMouseButtonPressed(PrCore::Events::EventPtr p_event);
		void OnMouseButtonReleased(PrCore::Events::EventPtr p_event);

		void OnMouseMoved(PrCore::Events::EventPtr p_event);
		void OnMouseScroll(PrCore::Events::EventPtr p_event);


		static KeyStateMap s_keyState;
		static KeyHoldMap s_keyHold;
		static ButtonStateMap s_buttonState;
		static ButtonHoldMap s_buttonHold;
		
		static double s_mouseXPos;
		static double s_mouseYPos;
		//Vector2 m_mousePos;
		static double s_mouseScroll;

		static bool s_anyPressed;
		static bool s_anyHold;


	};
}