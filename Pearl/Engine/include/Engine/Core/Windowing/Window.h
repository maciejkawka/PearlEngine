#pragma once
#include"WindowContext.h"
#include"WindowSettings.h"

namespace PrCore::Windowing {

	class Window {
	public:
		virtual ~Window() {}

		virtual void PollEvents() = 0;
		virtual void SwapBuffers() = 0;

		virtual std::string GetTitle() = 0;
		virtual int GetWidth() = 0;
		virtual int GetHeight() = 0;

		virtual bool ShouldClose() = 0;

		virtual void SetVSync(bool p_vsync) = 0;
		virtual bool IsVSync() = 0;

		virtual void SetIcon(std::string p_path) = 0;
		
		//TODO
		//virtual void SetCoursorMode() = 0;
		//virtual GetCoursorMode() =0;

		//Virtual void SetCoursorShape() = 0;
		//virtual GetCoursorShape() =0;
	};
}