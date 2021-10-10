#include"Event.h"

namespace PrCore::Events {

	class WindowCloseEvent: public Event {
	public:
		virtual inline EventType GetType() { return s_type; }
		inline const static EventType s_type = 0x1cd2925a;
	};

	class WindowResizeEvent: public Event {
	public:
		WindowResizeEvent(float p_width, float p_height):
			m_width(p_width), m_height(p_height)
		{}

		float m_width;
		float m_height;

		virtual inline EventType GetType() { return s_type; }
		inline const static EventType s_type = 0x9162135b;
	};

	class WindowMinimalizeEvent : public Event {
	public:
		WindowMinimalizeEvent(int p_iconified)
		{
			if (p_iconified == 1)
				m_minimalized = true;
			else
				m_minimalized = false;
		}

		bool m_minimalized;

		virtual inline EventType GetType() { return s_type; }
		inline const static EventType s_type = 0x1e4fd88e;
	};
}
