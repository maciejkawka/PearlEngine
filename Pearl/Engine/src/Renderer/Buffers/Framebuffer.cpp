#include"Core/Common/pearl_pch.h"

#include"Renderer/Buffers/Framebuffer.h"
#include "Renderer/Core/RendererAPI.h"
#include "Renderer/OpenGL/GLFramebuffer.h"

using namespace PrRenderer;

FramebuffferPtr Framebufffer::Create(const FramebufferSettings& p_settings)
{
	switch (RendererAPI::GetGraphicsAPI())
	{
	case GraphicsAPI::OpenGL: return std::make_shared<OpenGL::GLFramebuffer>(p_settings);

	default:
	{
		PRLOG_ERROR("No such Graphics API");
		return nullptr;
	}
	}
}
