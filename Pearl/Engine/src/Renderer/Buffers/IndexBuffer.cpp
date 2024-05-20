#include"Core/Common/pearl_pch.h"

#include"Renderer/Buffers/IndexBuffer.h"

#include"Renderer/Core/RendererAPI.h"
#include"Renderer/OpenGl/GLIndexBuffer.h"

using namespace PrRenderer::Buffers;

IndexBufferPtr IndexBuffer::Create()
{
	switch (Core::RendererAPI::GetGraphicsAPI())
	{
	case Core::GraphicsAPI::OpenGL: return std::make_shared<OpenGL::GLIndexBuffer>();

	default:
	{
		PRLOG_ERROR("No such Graphics API");
		return nullptr;
	}
	}
}