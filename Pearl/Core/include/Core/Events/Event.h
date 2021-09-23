#pragma once
#include <stdint.h>

namespace PrCore::Events {

	using  EventType = uint32_t;

	enum class EventCategory {
		none = 0,
		mouseEvent,
		windowEvent,
		other
	};

	class Event {
	public:
		Event():
		m_name(nullptr) {}

		Event(char* p_name) :
			m_name(p_name) {}

		virtual ~Event() {}

		inline static EventType GetType() { return s_type; }
		virtual inline const char* GetName() const { return m_name; }
		virtual inline EventCategory GetCategory() const { return s_category; }

	protected:
		inline static EventType s_type = 0x00000000;
		inline static EventCategory s_category = EventCategory::none;
		char* m_name;

	}; 
	


}