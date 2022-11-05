#pragma once
#include"Core/Events/Event.h"
#include"Core/Utils/NonCopyable.h"
namespace PrCore::Entry {

	class AppContext;

	class Application: public Utils::NonCopyable {
	public:
		Application();

		virtual ~Application();

		void Run();

		void SubscribeEvents();

		virtual void PreFrame() = 0;

		virtual void OnFrame(float p_deltaTime) = 0;

		virtual void PostFrame() = 0;
	
	private:
		void OnWindowClose(PrCore::Events::EventPtr p_event);

		void OnWindowMinimalized(PrCore::Events::EventPtr p_event);

	protected:
		AppContext* m_appContext;
		bool m_shouldClose;
		bool m_minimalized;
	};
}