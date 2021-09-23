#pragma once
#include <stdint.h>

namespace PrCore::Events {

	using  EventType = uint32_t;
	using  EventID = unsigned int;

	enum class EventCategory {
		none = 0,
		mouseEvent,
		windowEvent,
		other,
		custom
	};

	class Event {
	public:
		Event() {}

		virtual ~Event() { s_eventID++; }


		virtual inline EventType GetType() = 0;
		virtual inline EventCategory GetCategory() = 0;

		inline EventID GetID() { return s_eventID; }

	private:
		inline static EventID s_eventID = 0;
	}; 
	


}