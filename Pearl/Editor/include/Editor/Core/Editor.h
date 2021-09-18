#pragma once
#include"Core/Entry/Application.h"
#include"Core/Entry/EntryPoint.h"

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