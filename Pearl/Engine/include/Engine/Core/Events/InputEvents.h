#pragma once
#include"Event.h"
#include"Core/Input/PrKey.h"
#include"Core/Input/PrKeyState.h"
#include"Core/Input/PrMouseButton.h"

namespace PrCore::Events {

	class KeyPressedEvent : public Event {
	public:
		KeyPressedEvent(int p_key)
		{
			m_key = (PrCore::Input::PrKey)p_key;
		}

		PrCore::Input::PrKey m_key;

		virtual inline EventType GetType() { return s_type; }
		inline const static EventType s_type = 0x80e66091;
	};

	class KeyReleasedEvent : public Event {
	public:
		KeyReleasedEvent(int p_key)
		{
			m_key = (PrCore::Input::PrKey)p_key;
		}

		PrCore::Input::PrKey m_key;

		virtual inline EventType GetType() { return s_type; }
		inline const static EventType s_type = 0x26dfe3a;
	};

	class MouseButtonPressedEvent : public Event {
	public:
		MouseButtonPressedEvent(int p_button)
		{
			m_button = (PrCore::Input::PrMouseButton)p_button;
		}

		PrCore::Input::PrMouseButton m_button;

		virtual inline EventType GetType() { return s_type; }
		inline const static EventType s_type = 0xb84be5a9;
	};

	class MouseButtonReleasedEvent : public Event {
	public:
		MouseButtonReleasedEvent(int p_button)
		{
			m_button = (PrCore::Input::PrMouseButton)p_button;
		}

		PrCore::Input::PrMouseButton m_button;

		virtual inline EventType GetType() { return s_type; }
		inline const static EventType s_type = 0xe4f62295;
	};

	class MouseMovedEvent : public Event {
	public:
		MouseMovedEvent(double p_xPos, double p_yPos) :
			m_xPos(p_xPos),
			m_yPos(p_yPos)
		{}

		double m_xPos;
		double m_yPos;

		virtual inline EventType GetType() { return s_type; }
		inline const static EventType s_type = 0x57ac3e95;
	};

	class MouseScrollEvent : public Event {
	public:
		MouseScrollEvent(double p_xOffset, double p_yOffset) :
			m_xOffset(p_xOffset),
			m_yOffset(p_yOffset)
		{}

		double m_xOffset;
		double m_yOffset;

		virtual inline EventType GetType() { return s_type; }
		inline const static EventType s_type = 0x934ff217;
	};
}
