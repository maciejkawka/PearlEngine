#pragma once
#include <stdint.h>
#include<memory>

namespace PrCore::Events {
	
	using  EventType = uint32_t;
	using  EventID = unsigned int;
	
	class Event {
	public:
		Event() {}

		virtual ~Event() { s_eventID++; }

		virtual inline EventType GetType() = 0;

		inline EventID GetID() { return s_eventID; }

	private:
		inline static EventID s_eventID = 0;
	};
	
	typedef std::shared_ptr<Event> EventPtr;
}