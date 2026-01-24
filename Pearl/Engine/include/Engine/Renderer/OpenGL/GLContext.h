#pragma once
#include"Renderer/Core/ContextSettings.h"
#include"Renderer/Core/Context.h"

namespace PrRenderer::OpenGL {

	class GLContext : public Context {
	public:
		GLContext(const ContextSettings& p_contextSettings = ContextSettings());
		virtual ~GLContext();

		void Init() override;
	};
}