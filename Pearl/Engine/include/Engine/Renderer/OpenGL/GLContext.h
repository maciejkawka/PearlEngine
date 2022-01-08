#pragma once
#include"Renderer/Core/ContextSettings.h"
#include"Renderer/Core/Context.h"

namespace PrRenderer::OpenGL {

	class GLContext : public Core::Context {
	public:
		GLContext(const Core::ContextSettings& p_contextSettings = Core::ContextSettings());

		~GLContext() {}

		void Init() override;
	};
}