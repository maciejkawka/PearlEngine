#pragma once

#include "Engine/Core/Entry/IContext.h"
#include "Engine/Core/Events/Event.h"

#include "Editor/Components/BasicCamera.h"
#include "Editor/Components/TestFeatures.h"

namespace PrEditor {

	class EditorContext : public PrCore::IContext {
	public:
		bool OnInitalize(const PrCore::EngineCoreParams& p_params) override;
		void OnTerminate() override;
		bool OnUpdate(float p_dt) override;

		bool ShouldClose() override;

	private:
		void OnWindowClose(PrCore::EventPtr p_event);

		Components::BasicCamera* m_basicCamera;
		Components::TestFeatures* m_testFeatures;

		bool m_shouldClose = false;
	};
}