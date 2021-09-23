#include"Core/Events/Event.h"
#include"Core/Utils/Logger.h"
namespace PrCore::Events {

	class TestEvent:public Event {
	public:
		TestEvent() { s_type = 0x00000001; }
		~TestEvent() {}

		inline void Print() { PRLOG_INFO("TEST EVENT!"); }
		inline int Add(int a, int b) { return a + b; }
	};


	class TestEvent1 :public Event {
	public:
		TestEvent1() { s_type = 0x00000002; }
		~TestEvent1() {}

		inline void Print() { PRLOG_INFO("TEST EVENT!"); }
		inline int Add(int a, int b) { return a + b; }
	};
}