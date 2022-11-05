#include"Core/Common/pearl_pch.h"

#include"Core/Input/InputManager.h"
#include"Core/Events/EventManager.h"
#include"Core/Events/InputEvents.h"

using namespace PrCore::Input;

InputManager::InputManager()
{
	Events::EventListener keyPressedListener;
	keyPressedListener.connect<&InputManager::OnKeyPressed>(this);
	Events::EventManager::GetInstance().AddListener(keyPressedListener, Events::KeyPressedEvent::s_type);

	Events::EventListener keyReleasedListener;
	keyReleasedListener.connect<&InputManager::OnKeyReleased>(this);
	Events::EventManager::GetInstance().AddListener(keyReleasedListener, Events::KeyReleasedEvent::s_type);

	Events::EventListener buttonPressedListener;
	buttonPressedListener.connect<&InputManager::OnMouseButtonPressed>(this);
	Events::EventManager::GetInstance().AddListener(buttonPressedListener, Events::MouseButtonPressedEvent::s_type);

	Events::EventListener buttonReleasedListener;
	buttonReleasedListener.connect<&InputManager::OnMouseButtonReleased>(this);
	Events::EventManager::GetInstance().AddListener(buttonReleasedListener, Events::MouseButtonReleasedEvent::s_type);

	Events::EventListener mouseMovedListener;
	mouseMovedListener.connect<&InputManager::OnMouseMoved>(this);
	Events::EventManager::GetInstance().AddListener(mouseMovedListener, Events::MouseMovedEvent::s_type);

	Events::EventListener mouseScrollListener;
	mouseScrollListener.connect<&InputManager::OnMouseScroll>(this);
	Events::EventManager::GetInstance().AddListener(mouseScrollListener, Events::MouseScrollEvent::s_type);

	s_anyPressed = false;
	s_anyHold = false;
}

InputManager::~InputManager()
{
	Events::EventListener keyPressedListener;
	keyPressedListener.connect<&InputManager::OnKeyPressed>(this);
	Events::EventManager::GetInstance().RemoveListener(keyPressedListener, Events::KeyPressedEvent::s_type);

	Events::EventListener keyReleasedListener;
	keyReleasedListener.connect<&InputManager::OnKeyReleased>(this);
	Events::EventManager::GetInstance().RemoveListener(keyReleasedListener, Events::KeyReleasedEvent::s_type);

	Events::EventListener buttonPressedListener;
	buttonPressedListener.connect<&InputManager::OnMouseButtonPressed>(this);
	Events::EventManager::GetInstance().RemoveListener(buttonPressedListener, Events::MouseButtonPressedEvent::s_type);

	Events::EventListener buttonReleasedListener;
	buttonReleasedListener.connect<&InputManager::OnMouseButtonReleased>(this);
	Events::EventManager::GetInstance().RemoveListener(buttonReleasedListener, Events::MouseButtonReleasedEvent::s_type);

	Events::EventListener mouseMovedListener;
	mouseMovedListener.connect<&InputManager::OnMouseMoved>(this);
	Events::EventManager::GetInstance().RemoveListener(mouseMovedListener, Events::MouseMovedEvent::s_type);

	Events::EventListener mouseScrollListener;
	mouseScrollListener.connect<&InputManager::OnMouseScroll>(this);
	Events::EventManager::GetInstance().RemoveListener(mouseScrollListener, Events::MouseScrollEvent::s_type);
}

bool InputManager::IsKeyPressed(Input::PrKey p_key)
{
	auto keyIterator = s_keyState.find(p_key);
	if (keyIterator == s_keyState.end())
		return false;
	return keyIterator->second  == Input::PrInputState::PRESS;
}

bool InputManager::IsKeyReleased(Input::PrKey p_key)
{
	auto keyIterator = s_keyState.find(p_key);
	if (keyIterator == s_keyState.end())
		return false;
	return keyIterator->second == Input::PrInputState::RELEASE;
}

bool InputManager::IsKeyHold(Input::PrKey p_key)
{
	auto keyHoldIterator = s_keyHold.find(p_key);
	if (keyHoldIterator == s_keyHold.end())
		return false;
	return keyHoldIterator->second;
}

bool InputManager::IsButtonPressed(Input::PrMouseButton p_key)
{
	auto buttonIterator = s_buttonState.find(p_key);
	if (buttonIterator == s_buttonState.end())
		return false;
	return buttonIterator->second == Input::PrInputState::PRESS;
}

bool InputManager::IsButtonReleased(Input::PrMouseButton p_key)
{
	auto buttonIterator = s_buttonState.find(p_key);
	if (buttonIterator == s_buttonState.end())
		return false;
	return buttonIterator->second == Input::PrInputState::RELEASE;
}

bool InputManager::IsButtonHold(Input::PrMouseButton p_key)
{
	auto buttonHoldIterator = s_buttonHold.find(p_key);
	if (buttonHoldIterator == s_buttonHold.end())
		return false;
	return buttonHoldIterator->second;
}

bool InputManager::IsAnyKeyPressed()
{
	return s_anyPressed;
}

bool InputManager::IsAnyKeyHold()
{
	return s_anyHold;
}

double InputManager::GetMouseScroll()
{
	return s_mouseScroll;
}

double InputManager::GetMouseX()
{
	return s_mouseXPos;
}

double InputManager::GetMouseY()
{
	return s_mouseYPos;
}

PrCore::Math::vec2 InputManager::GetMousePosition()
{
	return Math::vec2(GetMouseX(), GetMouseY());
}

void InputManager::ResetFlags()
{
	s_anyPressed = false;
	for (auto it = s_keyState.begin(); it != s_keyState.end(); ++it)
 		s_anyPressed |= (it->second == Input::PrInputState::PRESS);

	s_anyHold = false;
	for (auto it = s_keyHold.begin(); it != s_keyHold.end(); ++it)
		s_anyHold |= it->second;
	
	s_keyState.clear();
	s_buttonState.clear();
}

void InputManager::OnKeyPressed(Events::EventPtr p_event)
{
	auto keyPresedEvent = std::static_pointer_cast<Events::KeyPressedEvent>(p_event);
	s_keyState[keyPresedEvent->m_key] = Input::PrInputState::PRESS;
	s_keyHold[keyPresedEvent->m_key] = true;
}

void InputManager::OnKeyReleased(Events::EventPtr p_event)
{
	auto keyReleasedEvent = std::static_pointer_cast<Events::KeyReleasedEvent>(p_event);
	s_keyState[keyReleasedEvent->m_key] = Input::PrInputState::RELEASE;
	s_keyHold[keyReleasedEvent->m_key] = false;
}

void InputManager::OnMouseButtonPressed(Events::EventPtr p_event)
{
	auto buttonPresedEvent = std::static_pointer_cast<Events::MouseButtonPressedEvent>(p_event);
	s_buttonState[buttonPresedEvent->m_button] = Input::PrInputState::PRESS;
	s_buttonHold[buttonPresedEvent->m_button] = true;
}

void InputManager::OnMouseButtonReleased(Events::EventPtr p_event)
{
	auto buttonReleasedEvent = std::static_pointer_cast<Events::MouseButtonReleasedEvent>(p_event);
	s_buttonState[buttonReleasedEvent->m_button] = Input::PrInputState::RELEASE;
	s_buttonHold[buttonReleasedEvent->m_button] = false;
}

void InputManager::OnMouseMoved(Events::EventPtr p_event)
{
	auto mouseMovedEvent = std::static_pointer_cast<Events::MouseMovedEvent>(p_event);
	s_mouseXPos = mouseMovedEvent->m_xPos;
	s_mouseYPos = mouseMovedEvent->m_yPos;
}

void InputManager::OnMouseScroll(Events::EventPtr p_event)
{
	auto mouseScrollEvent = std::static_pointer_cast<Events::MouseScrollEvent>(p_event);
	s_mouseScroll = mouseScrollEvent->m_yOffset;
}
