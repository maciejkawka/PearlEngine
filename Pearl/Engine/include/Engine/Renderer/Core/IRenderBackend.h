#pragma once
#include"Renderer/Core/RenderTypes.h"
#include"Renderer/Core/RenderCommand.h"

#include"Core/Events/Event.h"
#include<deque>

namespace PrRenderer::Core {

	class IRenderBackend {
	public:
		explicit IRenderBackend(RendererSettingsPtr& p_settings) :
			m_settings(p_settings),
			m_screenWidth(0),
			m_screenHeight(0)
		{}

		IRenderBackend() = default;
		virtual ~IRenderBackend() = default;

		inline void SetFrame(FrameDataPtr p_frame) { m_frame = p_frame; }
		inline void PushCommand(RenderCommandPtr p_command) { m_commandQueue.push_back(p_command); }

		inline RendererSettingsPtr GetSettingsPtr() { return m_settings; }
		inline RendererSettings& GetSettings() const { return *m_settings; }

		virtual void PreparePipeline() = 0;
		virtual void Render() = 0;
		virtual void PostRender() = 0;

	protected:
		using RenderCommandQueue = std::deque<RenderCommandPtr>;

		RenderCommandQueue    m_commandQueue;
		FrameDataPtr	      m_frame;
		RendererSettingsPtr   m_settings;

		size_t m_screenWidth;
		size_t m_screenHeight;
	};

	using RendererBackendPtr = std::shared_ptr<IRenderBackend>;
}