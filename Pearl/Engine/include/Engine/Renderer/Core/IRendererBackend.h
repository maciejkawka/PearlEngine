#pragma once
#include"Renderer/Core/RenderTypes.h"
#include"Renderer/Core/RenderCommand.h"
#include<queue>

namespace PrRenderer::Core {

	class IRendererBackend {
	public:
		explicit IRendererBackend(const RendererSettings& p_settings):
		m_settings(p_settings)
		{}
		virtual ~IRendererBackend() = default;

		inline void SetFrame(FrameDataPtr p_frame) { m_frame = p_frame; }
		inline void PushCommand(RenderCommandPtr p_command) { m_renderCommand.push_back(p_command); }

		inline RendererSettings GetSettings() const { return m_settings; }
		inline void SetSettings(const RendererSettings p_settings) { m_settings = p_settings; }

		virtual void Prerender() = 0;
		virtual void Render() = 0;
		virtual void PostRender() = 0;
		virtual void CleanUp() = 0;

	protected:
		using RenderCommand = std::deque<RenderCommandPtr>;
		RendererSettings m_settings;
		RenderCommand    m_renderCommand;
		FrameDataPtr	 m_frame;
	};

	using RendererBackendPtr = std::shared_ptr<IRendererBackend>;
}