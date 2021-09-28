#include"Core/Events/Event.h"


namespace PrCore::Events {

	class OnEvent: public Event {
	public:
		virtual inline EventType GetType() { return s_type; }
		inline const static EventType s_type = 0xaa0d1ba7;
	};
}
