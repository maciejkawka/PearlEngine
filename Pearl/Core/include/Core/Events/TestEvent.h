#include"Core/Events/Event.h"
#include"Core/Utils/Logger.h"
namespace PrCore::Events {

	class TestEvent:public Event {
	public:
		TestEvent() {}
		~TestEvent() {}

		virtual inline EventType GetType() { return s_type; }
		virtual inline EventCategory GetCategory() { return s_category; }

		inline void Print() { PRLOG_INFO("TEST EVENT!"); }
		inline int Add(int a, int b) { return a + b; }

		inline static EventType s_type = 0x00000001;
		inline static EventCategory s_category = EventCategory::mouseEvent;
	};


	class TestEvent1 :public Event {
	public:
		TestEvent1() {}
		~TestEvent1() {}

		virtual inline EventType GetType() { return s_type; }
		virtual inline EventCategory GetCategory() { return s_category; }

		inline void Print() { PRLOG_INFO("TEST EVENT!"); }
		inline int Add(int a, int b) { return a + b; }

		inline static EventType s_type = 0x00000002;
		inline static EventCategory s_category = EventCategory::windowEvent;
	};
}