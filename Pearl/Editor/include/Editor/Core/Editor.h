#pragma once
#include"Core/Entry/Application.h"
#include"Core/Events/Event.h"

namespace PrEditor::Core {

	class Editor : public PrCore::Entry::Application {
	public:
		Editor();
		~Editor();

		void PreFrame() override;

		void OnFrame() override;

		void PostFrame() override;
		
		void OnEvent(PrCore::Events::Event* e);
	};

}