#include"Core/Common/pearl_pch.h"

#include"Core/Input/InputManager.h"
#include"Core/Events/EventManager.h"
#include"Core/Events/InputEvents.h"

using namespace PrCore::Input;

InputManager::InputManager() :
	s_mouseXPos(0.0f),
	s_mouseYPos(0.0f),
	s_mouseScroll(0.0f),
	s_anyPressed(false),
	s_anyHold(false)
{
	EventListener keyPressedListener;
	keyPressedListener.Connect<&InputManager::OnKeyPressed>(this);
	PrSystems::Get<EventManager>()->AddListener(keyPressedListener, KeyPressedEvent::s_type);

	EventListener keyReleasedListener;
	keyReleasedListener.Connect<&InputManager::OnKeyReleased>(this);
	PrSystems::Get<EventManager>()->AddListener(keyReleasedListener, KeyReleasedEvent::s_type);

	EventListener buttonPressedListener;
	buttonPressedListener.Connect<&InputManager::OnMouseButtonPressed>(this);
	PrSystems::Get<EventManager>()->AddListener(buttonPressedListener, MouseButtonPressedEvent::s_type);

	EventListener buttonReleasedListener;
	buttonReleasedListener.Connect<&InputManager::OnMouseButtonReleased>(this);
	PrSystems::Get<EventManager>()->AddListener(buttonReleasedListener, MouseButtonReleasedEvent::s_type);

	EventListener mouseMovedListener;
	mouseMovedListener.Connect<&InputManager::OnMouseMoved>(this);
	PrSystems::Get<EventManager>()->AddListener(mouseMovedListener, MouseMovedEvent::s_type);

	EventListener mouseScrollListener;
	mouseScrollListener.Connect<&InputManager::OnMouseScroll>(this);
	PrSystems::Get<EventManager>()->AddListener(mouseScrollListener, MouseScrollEvent::s_type);
}

InputManager::~InputManager()
{
	EventListener keyPressedListener;
	keyPressedListener.Connect<&InputManager::OnKeyPressed>(this);
	PrSystems::Get<EventManager>()->RemoveListener(keyPressedListener, KeyPressedEvent::s_type);

	EventListener keyReleasedListener;
	keyReleasedListener.Connect<&InputManager::OnKeyReleased>(this);
	PrSystems::Get<EventManager>()->RemoveListener(keyReleasedListener, KeyReleasedEvent::s_type);

	EventListener buttonPressedListener;
	buttonPressedListener.Connect<&InputManager::OnMouseButtonPressed>(this);
	PrSystems::Get<EventManager>()->RemoveListener(buttonPressedListener, MouseButtonPressedEvent::s_type);

	EventListener buttonReleasedListener;
	buttonReleasedListener.Connect<&InputManager::OnMouseButtonReleased>(this);
	PrSystems::Get<EventManager>()->RemoveListener(buttonReleasedListener, MouseButtonReleasedEvent::s_type);

	EventListener mouseMovedListener;
	mouseMovedListener.Connect<&InputManager::OnMouseMoved>(this);
	PrSystems::Get<EventManager>()->RemoveListener(mouseMovedListener, MouseMovedEvent::s_type);

	EventListener mouseScrollListener;
	mouseScrollListener.Connect<&InputManager::OnMouseScroll>(this);
	PrSystems::Get<EventManager>()->RemoveListener(mouseScrollListener, MouseScrollEvent::s_type);
}

bool InputManager::IsKeyPressed(PrKey p_key) const
{
	auto keyIterator = s_keyState.find(p_key);
	if (keyIterator == s_keyState.end())
		return false;
	return keyIterator->second  == PrInputState::PRESS;
}

bool InputManager::IsKeyReleased(PrKey p_key) const
{
	auto keyIterator = s_keyState.find(p_key);
	if (keyIterator == s_keyState.end())
		return false;
	return keyIterator->second == PrInputState::RELEASE;
}

bool InputManager::IsKeyHold(PrKey p_key) const
{
	auto keyHoldIterator = s_keyHold.find(p_key);
	if (keyHoldIterator == s_keyHold.end())
		return false;
	return keyHoldIterator->second;
}

bool InputManager::IsButtonPressed(PrMouseButton p_key) const
{
	auto buttonIterator = s_buttonState.find(p_key);
	if (buttonIterator == s_buttonState.end())
		return false;
	return buttonIterator->second == PrInputState::PRESS;
}

bool InputManager::IsButtonReleased(PrMouseButton p_key) const
{
	auto buttonIterator = s_buttonState.find(p_key);
	if (buttonIterator == s_buttonState.end())
		return false;
	return buttonIterator->second == PrInputState::RELEASE;
}

bool InputManager::IsButtonHold(PrMouseButton p_key) const
{
	auto buttonHoldIterator = s_buttonHold.find(p_key);
	if (buttonHoldIterator == s_buttonHold.end())
		return false;
	return buttonHoldIterator->second;
}

bool InputManager::IsAnyKeyPressed() const
{
	return s_anyPressed;
}

bool InputManager::IsAnyKeyHold() const
{
	return s_anyHold;
}

double InputManager::GetMouseScroll() const
{
	return s_mouseScroll;
}

double InputManager::GetMouseX() const
{
	return s_mouseXPos;
}

double InputManager::GetMouseY() const
{
	return s_mouseYPos;
}

PrCore::Math::vec2 InputManager::GetMousePosition() const
{
	return Math::vec2(GetMouseX(), GetMouseY());
}

void InputManager::ResetFlags()
{
	s_anyPressed = false;
	for (auto it = s_keyState.begin(); it != s_keyState.end(); ++it)
 		s_anyPressed |= (it->second == PrInputState::PRESS);

	s_anyHold = false;
	for (auto it = s_keyHold.begin(); it != s_keyHold.end(); ++it)
		s_anyHold |= it->second;
	
	s_keyState.clear();
	s_buttonState.clear();
}

void InputManager::OnKeyPressed(EventPtr p_event)
{
	auto keyPresedEvent = std::static_pointer_cast<KeyPressedEvent>(p_event);
	s_keyState[keyPresedEvent->m_key] = PrInputState::PRESS;
	s_keyHold[keyPresedEvent->m_key] = true;
}

void InputManager::OnKeyReleased(EventPtr p_event)
{
	auto keyReleasedEvent = std::static_pointer_cast<KeyReleasedEvent>(p_event);
	s_keyState[keyReleasedEvent->m_key] = PrInputState::RELEASE;
	s_keyHold[keyReleasedEvent->m_key] = false;
}

void InputManager::OnMouseButtonPressed(EventPtr p_event)
{
	auto buttonPresedEvent = std::static_pointer_cast<MouseButtonPressedEvent>(p_event);
	s_buttonState[buttonPresedEvent->m_button] = PrInputState::PRESS;
	s_buttonHold[buttonPresedEvent->m_button] = true;
}

void InputManager::OnMouseButtonReleased(EventPtr p_event)
{
	auto buttonReleasedEvent = std::static_pointer_cast<MouseButtonReleasedEvent>(p_event);
	s_buttonState[buttonReleasedEvent->m_button] = PrInputState::RELEASE;
	s_buttonHold[buttonReleasedEvent->m_button] = false;
}

void InputManager::OnMouseMoved(EventPtr p_event)
{
	auto mouseMovedEvent = std::static_pointer_cast<MouseMovedEvent>(p_event);
	s_mouseXPos = mouseMovedEvent->m_xPos;
	s_mouseYPos = mouseMovedEvent->m_yPos;
}

void InputManager::OnMouseScroll(EventPtr p_event)
{
	auto mouseScrollEvent = std::static_pointer_cast<MouseScrollEvent>(p_event);
	s_mouseScroll = mouseScrollEvent->m_yOffset;
}
