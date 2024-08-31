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

		virtual inline EventType GetType() const = 0;

		inline EventID GetID() const { return s_eventID; }

	private:
		inline static EventID s_eventID = 0;
	};
	
	typedef std::shared_ptr<Event> EventPtr;

#define DEFINE_EVENT_GUID(guid) \
	virtual inline EventType GetType() const { return s_type; } \
	inline const static EventType s_type = guid

}