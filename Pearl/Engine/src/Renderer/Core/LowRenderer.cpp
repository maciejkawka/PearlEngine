#include"Core/Common/pearl_pch.h"

#include"Renderer/Core/LowRenderer.h"
#include"Renderer/OpenGL/GLRenderer.h"

using namespace PrRenderer::Core;

void LowRenderer::Init(GraphicsAPI p_API)
{
	switch (p_API)
	{
	case GraphicsAPI::OpenGL:
		m_rendererAPI = new OpenGL::GLRenderer();
		break;
	case GraphicsAPI::DirectX:
		break;
	default:
		PRLOG_ERROR("Wrong GraphicsAPI");
		return;
	}
}

void LowRenderer::Terminate()
{
	if (m_rendererAPI)
		delete m_rendererAPI;
}

void LowRenderer::Clear(ClearFlag p_flag)
{
	m_rendererAPI->Clear(p_flag);
}

void LowRenderer::ClearColor(float p_r, float p_g, float p_b, float p_a)
{
	m_rendererAPI->ClearColor(p_r, p_g, p_b, p_a);
}

void LowRenderer::ClearColor(Color& p_color)
{
	m_rendererAPI->ClearColor(p_color);
}

void LowRenderer::SetViewport(int p_width, int p_height, int p_x, int p_y)
{
	m_rendererAPI->SetViewport(p_width, p_height, p_x, p_y);
}

void LowRenderer::SetViewportArray(int p_first, int p_count, float* p_viewportData)
{
	m_rendererAPI->SetViewportArray(p_first, p_count, p_viewportData);
}

void LowRenderer::GetViewport(int& p_width, int& p_height, int& p_x, int& p_y)
{
	m_rendererAPI->GetViewport(p_width, p_height, p_x, p_y);
}

void LowRenderer::EnableDepth(bool p_enable)
{
	m_rendererAPI->EnableDepth(p_enable);
}

void LowRenderer::SetDepthTest(bool p_enable)
{
	m_rendererAPI->SetDepthTest(p_enable);
}

void LowRenderer::SetDepthAlgorythm(ComparaisonAlgorithm p_algorythm)
{
	m_rendererAPI->SetDepthAlgorythm(p_algorythm);
}

void LowRenderer::EnableBlending(bool p_enable)
{
	m_rendererAPI->EnableBlending(p_enable);
}

void LowRenderer::SetBlendingAlgorythm(BlendingAlgorithm p_source, BlendingAlgorithm p_dest)
{
	m_rendererAPI->SetBlendingAlgorythm(p_source, p_dest);
}

void LowRenderer::SetColorMask(bool p_red, bool p_green, bool p_blue, bool p_alpha)
{
	m_rendererAPI->SetColorMask(p_red, p_green, p_blue, p_alpha);
}

void LowRenderer::Draw(Buffers::VertexArrayPtr p_vertexArray, size_t p_indicesCount , unsigned int p_indicesOffset, Core::Primitives p_primitives)
{
	m_rendererAPI->Draw(p_vertexArray, p_indicesCount, p_indicesOffset, p_primitives);
}

void LowRenderer::DrawArray(Buffers::VertexBufferPtr p_vertexArray, Core::Primitives p_primitives)
{
	m_rendererAPI->DrawArray(p_vertexArray, p_primitives);
}

void LowRenderer::DrawInstanced(Buffers::VertexArrayPtr p_vertexArray, size_t p_instanceCount, Primitives p_primitives)
{
	m_rendererAPI->DrawInstanced(p_vertexArray, p_instanceCount, p_primitives);
}

void LowRenderer::EnableCullFace(bool p_enable)
{
	m_rendererAPI->EnableCullFace(p_enable);
}

void LowRenderer::SetCullFaceMode(CullFaceMode p_mode)
{
	m_rendererAPI->SetCullFaceMode(p_mode);
}

void LowRenderer::BlitFrameBuffers(Buffers::FramebuffferPtr p_readBuffer, Buffers::FramebuffferPtr p_drawBuffer, Buffers::FramebufferMask p_mask)
{
	m_rendererAPI->BlitFrameBuffers(p_readBuffer, p_drawBuffer, p_mask);
}