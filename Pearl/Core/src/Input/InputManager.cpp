#include"Core/Common/pearl_pch.h"

#include"Core/Input/InputManager.h"
#include"Core/Events/EventManager.h"
#include"Core/Events/InputEvents.h"


using namespace PrCore::Input;

InputManager::KeyStateMap		InputManager::s_keyState;
InputManager::KeyHoldMap		InputManager::s_keyHold;
InputManager::ButtonStateMap	InputManager::s_buttonState;
InputManager::ButtonHoldMap		InputManager::s_buttonHold;

double InputManager::s_mouseXPos;
double InputManager::s_mouseYPos;
double InputManager::s_mouseScroll;

bool InputManager::s_anyPressed = false;
bool InputManager::s_anyHold = false;

InputManager::InputManager()
{
	PrCore::Events::EventListener keyPressedListener;
	keyPressedListener.connect<&InputManager::OnKeyPressed>(this);
	PrCore::Events::EventManager::GetInstance().AddListener(keyPressedListener, PrCore::Events::KeyPressedEvent::s_type);

	PrCore::Events::EventListener keyReleasedListener;
	keyReleasedListener.connect<&InputManager::OnKeyReleased>(this);
	PrCore::Events::EventManager::GetInstance().AddListener(keyReleasedListener, PrCore::Events::KeyReleasedEvent::s_type);

	PrCore::Events::EventListener buttonPressedListener;
	buttonPressedListener.connect<&InputManager::OnMouseButtonPressed>(this);
	PrCore::Events::EventManager::GetInstance().AddListener(buttonPressedListener, PrCore::Events::MouseButtonPressedEvent::s_type);

	PrCore::Events::EventListener buttonReleasedListener;
	buttonReleasedListener.connect<&InputManager::OnMouseButtonReleased>(this);
	PrCore::Events::EventManager::GetInstance().AddListener(buttonReleasedListener, PrCore::Events::MouseButtonReleasedEvent::s_type);

	PrCore::Events::EventListener mouseMovedListener;
	mouseMovedListener.connect<&InputManager::OnMouseMoved>(this);
	PrCore::Events::EventManager::GetInstance().AddListener(mouseMovedListener, PrCore::Events::MouseMovedEvent::s_type);

	PrCore::Events::EventListener mouseScrollListener;
	mouseScrollListener.connect<&InputManager::OnMouseScroll>(this);
	PrCore::Events::EventManager::GetInstance().AddListener(mouseScrollListener, PrCore::Events::MouseScrollEvent::s_type);
}

PrCore::Input::InputManager::~InputManager()
{
	//PrCore::Events::EventListener keyPressedListener;
	//keyPressedListener.connect<&InputManager::OnKeyPressed>(this);
	//PrCore::Events::EventManager::GetInstance().RemoveListener(keyPressedListener, PrCore::Events::KeyPressedEvent::s_type);

	//PrCore::Events::EventListener keyReleasedListener;
	//keyReleasedListener.connect<&InputManager::OnKeyReleased>(this);
	//PrCore::Events::EventManager::GetInstance().RemoveListener(keyReleasedListener, PrCore::Events::KeyReleasedEvent::s_type);

	//PrCore::Events::EventListener buttonPressedListener;
	//buttonPressedListener.connect<&InputManager::OnMouseButtonPressed>(this);
	//PrCore::Events::EventManager::GetInstance().RemoveListener(buttonPressedListener, PrCore::Events::MouseButtonPressedEvent::s_type);

	//PrCore::Events::EventListener buttonReleasedListener;
	//buttonReleasedListener.connect<&InputManager::OnMouseButtonReleased>(this);
	//PrCore::Events::EventManager::GetInstance().RemoveListener(buttonReleasedListener, PrCore::Events::MouseButtonReleasedEvent::s_type);

	//PrCore::Events::EventListener mouseMovedListener;
	//mouseMovedListener.connect<&InputManager::OnMouseMoved>(this);
	//PrCore::Events::EventManager::GetInstance().RemoveListener(mouseMovedListener, PrCore::Events::MouseMovedEvent::s_type);

	//PrCore::Events::EventListener mouseScrollListener;
	//mouseScrollListener.connect<&InputManager::OnMouseScroll>(this);
	//PrCore::Events::EventManager::GetInstance().RemoveListener(mouseScrollListener, PrCore::Events::MouseScrollEvent::s_type);
}

bool InputManager::IsKeyPressed(PrCore::Input::PrKey p_key)
{
	auto keyIterator = s_keyState.find(p_key);
	if (keyIterator == s_keyState.end())
		return false;
	return keyIterator->second  == PrCore::Input::PrInputState::PRESS;
}

bool InputManager::IsKeyReleased(PrCore::Input::PrKey p_key)
{
	auto keyIterator = s_keyState.find(p_key);
	if (keyIterator == s_keyState.end())
		return false;
	return keyIterator->second == PrCore::Input::PrInputState::RELEASE;
}

bool PrCore::Input::InputManager::IsKeyHold(PrCore::Input::PrKey p_key)
{
	auto keyHoldIterator = s_keyHold.find(p_key);
	if (keyHoldIterator == s_keyHold.end())
		return false;
	return keyHoldIterator->second;
}

bool InputManager::IsButtonPressed(PrCore::Input::PrMouseButton p_key)
{
	auto buttonIterator = s_buttonState.find(p_key);
	if (buttonIterator == s_buttonState.end())
		return false;
	return buttonIterator->second == PrCore::Input::PrInputState::PRESS;
}

bool PrCore::Input::InputManager::IsButtonReleased(PrCore::Input::PrMouseButton p_key)
{
	auto buttonIterator = s_buttonState.find(p_key);
	if (buttonIterator == s_buttonState.end())
		return false;
	return buttonIterator->second == PrCore::Input::PrInputState::RELEASE;
}

bool PrCore::Input::InputManager::IsButtonHold(PrCore::Input::PrMouseButton p_key)
{
	auto buttonHoldIterator = s_buttonHold.find(p_key);
	if (buttonHoldIterator == s_buttonHold.end())
		return false;
	return buttonHoldIterator->second;
}

bool PrCore::Input::InputManager::IsAnyKeyPressed()
{
	if (s_keyState.empty())
		return false;

	bool isAnyPressed = true;
	for (auto it = s_keyState.begin(); it != s_keyState.end(); ++it)
		isAnyPressed &= (it->second == PrCore::Input::PrInputState::PRESS);

	return isAnyPressed;
}

void InputManager::ResetFlag()
{
	s_keyState.clear();
	s_buttonState.clear();
}

void InputManager::OnKeyPressed(PrCore::Events::EventPtr p_event)
{
	auto keyPresedEvent = std::static_pointer_cast<PrCore::Events::KeyPressedEvent>(p_event);
	s_keyState[keyPresedEvent->m_key] = PrCore::Input::PrInputState::PRESS;
	s_keyHold[keyPresedEvent->m_key] = true;
}

void InputManager::OnKeyReleased(PrCore::Events::EventPtr p_event)
{
	auto keyReleasedEvent = std::static_pointer_cast<PrCore::Events::KeyReleasedEvent>(p_event);
	s_keyState[keyReleasedEvent->m_key] = PrCore::Input::PrInputState::RELEASE;
	s_keyHold[keyReleasedEvent->m_key] = false;
}

void InputManager::OnMouseButtonPressed(PrCore::Events::EventPtr p_event)
{
	auto buttonPresedEvent = std::static_pointer_cast<PrCore::Events::MouseButtonPressedEvent>(p_event);
	s_buttonState[buttonPresedEvent->m_button] = PrCore::Input::PrInputState::PRESS;
	s_buttonHold[buttonPresedEvent->m_button] = true;
}

void InputManager::OnMouseButtonReleased(PrCore::Events::EventPtr p_event)
{
	auto buttonReleasedEvent = std::static_pointer_cast<PrCore::Events::MouseButtonReleasedEvent>(p_event);
	s_buttonState[buttonReleasedEvent->m_button] = PrCore::Input::PrInputState::RELEASE;
	s_buttonHold[buttonReleasedEvent->m_button] = false;
}

void InputManager::OnMouseMoved(PrCore::Events::EventPtr p_event)
{
	auto mouseMovedEvent = std::static_pointer_cast<PrCore::Events::MouseMovedEvent>(p_event);
	s_mouseXPos = mouseMovedEvent->m_xPos;
	s_mouseYPos = mouseMovedEvent->m_yPos;
}

void InputManager::OnMouseScroll(PrCore::Events::EventPtr p_event)
{
	auto mouseScrollEvent = std::static_pointer_cast<PrCore::Events::MouseScrollEvent>(p_event);
	s_mouseScroll = mouseScrollEvent->m_yOffset;
}
