#pragma once


namespace PrCore::Entry {

	class AppContext;

	class Application {
	public:
		Application();

		virtual ~Application();

		void Run();

		virtual void PreFrame() = 0;

		virtual void OnFrame(/*pass deltaTime*/) = 0;

		virtual void PostFrame() = 0;
	
	private:
		bool IsRunning();

		bool IsMinimalised();

	protected:
	//TODO
	//check why it doesn't work
	AppContext* m_appContext;
	};
}