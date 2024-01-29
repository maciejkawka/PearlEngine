#pragma once
#include"Renderer/Core/RenderTypes.h"
#include"Renderer/Core/RenderCommand.h"

#include"Core/Events/Event.h"
#include<deque>

namespace PrRenderer::Core {

	class IRendererBackend {
	public:
		explicit IRendererBackend(RendererSettingsPtr& p_settings) :
			m_settings(p_settings),
			m_screenWidth(0),
			m_screenHeight(0)
		{}

		IRendererBackend() = default;
		virtual ~IRendererBackend() = default;

		inline void SetFrame(FrameDataPtr p_frame) { m_frame = p_frame; }
		inline void PushCommand(RenderCommandPtr p_command) { m_commandQueue.push_back(p_command); }

		inline RendererSettingsPtr GetSettingsPtr() { return m_settings; }
		inline RendererSettings& GetSettings() const { return *m_settings; }

		virtual void PreRender() = 0;
		virtual void Render() = 0;
		virtual void PostRender() = 0;

	protected:
		using RenderCommandQueue = std::deque<RenderCommandPtr>;

		RenderCommandQueue    m_commandQueue;
		FrameDataPtr	      m_frame;
		RendererSettingsPtr   m_settings;

		float m_screenWidth;
		float m_screenHeight;
	};

	using RendererBackendPtr = std::shared_ptr<IRendererBackend>;

#define TIME_RC_START(label) \
	PrCore::Utils::HighResolutionTimer label; \
	PushCommand(CreateRC<LambdaFunctionRC>([&] { label.Start(); }))

#define TIME_RC_STOP(label) \
	PushCommand(CreateRC<LambdaFunctionRC>([&] { m_frame->frameInfo.timeEvents.push_back({ #label, label.Stop() }); }))

}