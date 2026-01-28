#pragma once

#include "Window.h"
#include "WindowContext.h"
#include "WindowSettings.h"

class GLFWwindow;

namespace PrCore {

	class GLWindow : public IWindow {
	public:
		GLWindow(const WindowContext& p_context, const WindowSettings& p_settings);
		virtual ~GLWindow();

		bool InitDevice(const WindowContext& p_context);
		void TerminateDevice();

		void PollEvents() override;
		void SwapBuffers() override;

		inline std::string GetTitle() const override { return m_settings.title; }
		inline int GetWidth() const override { return m_settings.width; }
		inline int GetHeight() const override { return m_settings.height; }

		bool ShouldClose() override;

		void SetVSync(bool p_vsync) override;
		inline bool IsVSync() const override { return m_settings.vSync; }

		void SetIcon(std::string p_path) override;

	private:
		void BindCallbacks();

		GLFWwindow*    m_window;
		WindowSettings m_settings{};
		WindowContext  m_context{};
	};
}