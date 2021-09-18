#pragma once
#include"Core/Entry/Application.h"


namespace PrEditor::Core {

	class Editor : public PrCore::Entry::Application {
	public:
		Editor();
		~Editor();

		void PreFrame() override;

		void OnFrame() override;

		void PostFrame() override;
		
	};

}