#include"Core/Common/pearl_pch.h"

#include"Renderer/Buffers/VertexArray.h"
#include"Renderer/Core/RendererAPI.h"
#include"Renderer/OpenGl/GLVertexArray.h"

using namespace PrRenderer;

VertexArrayPtr VertexArray::Create()
{
	switch (RendererAPI::GetGraphicsAPI())
	{
	case GraphicsAPI::OpenGL: return std::make_shared<OpenGL::GLVertexArray>();

	default:
	{
		PRLOG_ERROR("No such Graphics API");
		return nullptr;
	}
	}
}