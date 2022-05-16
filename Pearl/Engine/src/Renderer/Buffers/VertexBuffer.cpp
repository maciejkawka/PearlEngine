#include"Core/Common/pearl_pch.h"

#include"Renderer/OpenGL/GLVertexBuffer.h"

#include"Renderer/Buffers/VertexBuffer.h"
#include"Renderer/Core/RendererAPI.h"

using namespace PrRenderer::Buffers;

PrRenderer::VertexBufferPtr VertexBuffer::Create()
{
	switch (Core::RendererAPI::GetGraphicsAPI())
	{
	case Core::GraphicsAPI::OpenGL: return std::make_shared<OpenGL::GLVertexBuffer>();

	default:
	{
		PRLOG_ERROR("No such Graphics API");
		return nullptr;
	}
	}
}

PrRenderer::VertexBufferPtr VertexBuffer::Create(void* p_data, size_t p_size)
{
	switch (Core::RendererAPI::GetGraphicsAPI())
	{
	case Core::GraphicsAPI::OpenGL: return std::make_shared<OpenGL::GLVertexBuffer>(p_data, p_size);

	default:
	{
		PRLOG_ERROR("No such Graphics API");
		return nullptr;
	}
	}
}
