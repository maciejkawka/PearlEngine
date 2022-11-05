#pragma once
#include"PrKey.h"
#include"PrMouseButton.h"
#include"PrKeyState.h"

#include"Core/Utils/Singleton.h"
#include"Core/Events/Event.h"
#include"Core/Math/Math.h"

#include<unordered_map>

namespace PrCore::Input {

	class InputManager: public Utils::Singleton<InputManager> {

		typedef std::unordered_map<PrKey, PrInputState>				KeyStateMap;
		typedef std::unordered_map<PrKey, bool>						KeyHoldMap;
		typedef std::unordered_map<PrMouseButton, PrInputState>		ButtonStateMap;
		typedef std::unordered_map<PrMouseButton, bool>				ButtonHoldMap;

	public:
		bool IsKeyPressed(PrKey p_key);
		bool IsKeyReleased(PrKey p_key);
		bool IsKeyHold(PrKey p_key);

		bool IsButtonPressed(PrMouseButton p_key);
		bool IsButtonReleased(PrMouseButton p_key);
		bool IsButtonHold(PrMouseButton p_key);

		bool IsAnyKeyPressed();
		bool IsAnyKeyHold();

		double GetMouseScroll();
		double GetMouseX();
		double GetMouseY();
		Math::vec2 GetMousePosition();

		void ResetFlags();

	private:
		InputManager();
		~InputManager();

		void OnKeyPressed(Events::EventPtr p_event);
		void OnKeyReleased(Events::EventPtr p_event);

		void OnMouseButtonPressed(Events::EventPtr p_event);
		void OnMouseButtonReleased(Events::EventPtr p_event);

		void OnMouseMoved(Events::EventPtr p_event);
		void OnMouseScroll(Events::EventPtr p_event);

		KeyStateMap s_keyState;
		KeyHoldMap s_keyHold;
		ButtonStateMap s_buttonState;
		ButtonHoldMap s_buttonHold;
		
		double s_mouseXPos;
		double s_mouseYPos;
		double s_mouseScroll;

		bool s_anyPressed;
		bool s_anyHold;

		friend Singleton<InputManager>;
	};
}