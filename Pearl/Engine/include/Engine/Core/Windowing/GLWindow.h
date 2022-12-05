#pragma once
#include"Window.h"
#include"WindowContext.h"
#include"WindowSettings.h"

class GLFWwindow;

namespace PrCore::Windowing {

	class GLWindow : public Window {
	public:
		GLWindow(const WindowSettings& p_settings = WindowSettings());

		~GLWindow();

		static void InitDevice(const WindowContext& p_context);
		static void TerminateDevice();

		void PollEvents() override;
		void SwapBuffers() override;

		inline std::string GetTitle() override { return m_settings.title; }
		inline int GetWidth() override { return m_settings.width; }
		inline int GetHeight() override { return m_settings.height; }

		bool ShouldClose() override;

		void SetVSync(bool p_vsync) override;
		inline bool IsVSync() override { return m_settings.vSync; }

		void SetIcon(std::string p_path) override;

	private:
		void BindCallbacks();

		GLFWwindow* m_window;
		WindowSettings m_settings;

		static WindowContext s_context;
		static bool s_init;
		static int s_windowsCount;
	};
}