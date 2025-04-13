#pragma once
#include"Engine/Core/Entry/Application.h"
#include"Engine/Core/Events/Event.h"

#include"Editor/Components/BasicCamera.h"
#include"Editor/Components/TestFeatures.h"

namespace PrEditor::Core {

	class Editor : public PrCore::Entry::Application {
	public:
		Editor();
		~Editor();

		void PreFrame() override;

		void OnFrame(float p_deltaTime) override;

		void PostFrame() override;

	private:
		Components::BasicCamera* m_basicCamera;
		Components::TestFeatures* m_testFeatures;

		const float m_physicsFixStep = 1.0f / 30.0f;
		float       m_physicsStepAccumulator = 0.0f;
	};

}