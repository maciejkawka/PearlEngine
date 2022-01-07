#pragma once
#include"Engine/Core/Entry/Application.h"
#include"Engine/Core/Events/Event.h"

namespace PrEditor::Core {

	class Editor : public PrCore::Entry::Application {
	public:
		Editor();
		~Editor();

		void PreFrame() override;

		void OnFrame(float p_deltaTime) override;

		void PostFrame() override;
	};

}