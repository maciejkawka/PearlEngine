#pragma once

#include "Core/Utils/SystemProvider.h"

#include <string>

namespace PrCore {

	class IWindow : public Utils::ISystem {
	public:
		virtual ~IWindow() = default;

		virtual void PollEvents() = 0;
		virtual void SwapBuffers() = 0;

		virtual std::string GetTitle() const = 0;
		virtual int GetWidth() const = 0;
		virtual int GetHeight() const = 0;

		virtual bool ShouldClose() = 0;

		virtual void SetVSync(bool p_vsync) = 0;
		virtual bool IsVSync() const = 0;

		virtual void SetIcon(std::string p_path) = 0;

		//TODO
		//virtual void SetCoursorMode() = 0;
		//virtual GetCoursorMode() =0;

		//Virtual void SetCoursorShape() = 0;
		//virtual GetCoursorShape() =0;
	};
}