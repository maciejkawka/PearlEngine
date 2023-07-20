#pragma once
#include"Renderer/Core/RenderTypes.h"
#include"Renderer/Core/RenderCommand.h"

#include"Core/Events/Event.h"
#include<deque>

namespace PrRenderer::Core {

	class IRendererBackend {
	public:
		explicit IRendererBackend(const RendererSettings& p_settings) :
			m_settings(p_settings),
			m_screenWidth(0),
			m_screenHeight(0)
		{}

		IRendererBackend() = default;
		virtual ~IRendererBackend() = default;

		inline void SetFrame(FrameDataPtr p_frame) { m_frame = p_frame; }
		inline void PushCommand(RenderCommandPtr p_command) { m_commandQueue.push_back(p_command); }

		inline RendererSettings GetSettings() const { return m_settings; }
		inline void SetSettings(const RendererSettings p_settings) { m_settings = p_settings; }

		virtual void PreRender() = 0;
		virtual void Render() = 0;
		virtual void PostRender() = 0;

	protected:
		using RenderCommandQueue = std::deque<RenderCommandPtr>;
		RendererSettings m_settings;
		RenderCommandQueue    m_commandQueue;
		FrameDataPtr	 m_frame;

		float m_screenWidth;
		float m_screenHeight;
	};

	using RendererBackendPtr = std::shared_ptr<IRendererBackend>;
}