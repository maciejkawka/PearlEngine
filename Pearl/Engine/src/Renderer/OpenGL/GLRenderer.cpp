#include"Core/Common/pearl_pch.h"

#include "Renderer/OpenGL/GLRenderer.h"
#include "Renderer/OpenGL/GLUtils.h"
#include "Core/Windowing/Window.h"

#include"glad/glad.h"

using namespace PrRenderer::OpenGL;

GLRenderer::GLRenderer()
{
	RendererAPI::m_graphicsAPI = GraphicsAPI::OpenGL;
}

void GLRenderer::Clear(ClearFlag p_flag)
{
	int mask = 0;

	if (p_flag & ClearFlag::ColorBuffer)
		mask |= GL_COLOR_BUFFER_BIT;
	if (p_flag & ClearFlag::DepthBuffer)
		mask |= GL_DEPTH_BUFFER_BIT;
	if (p_flag & ClearFlag::StencilBuffer)
		mask |= GL_STENCIL_BUFFER_BIT;

	glClear(mask);
}

void GLRenderer::ClearColor(float p_r, float p_g, float p_b, float p_a)
{
	glClearColor(p_r, p_g, p_b, p_a);
}

void GLRenderer::ClearColor(Color& p_color)
{
	glClearColor(p_color.r, p_color.g, p_color.b, p_color.a);
}

void GLRenderer::SetViewport(int p_width, int p_height, int p_x, int p_y)
{
	glViewport(p_x, p_y, p_width, p_height);
}

void GLRenderer::SetViewportArray(int p_first, int p_count, float* p_viewportData)
{
	glViewportArrayv(p_first, p_count, p_viewportData);
}

void GLRenderer::GetViewport(int& p_width, int& p_height, int& p_x, int& p_y)
{
	GLint m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);
	p_x = m_viewport[0];
	p_y = m_viewport[1];
	p_width = m_viewport[2];
	p_height = m_viewport[3];
}

void GLRenderer::EnableDepth(bool p_enable)
{
	// Enable/Disable Writing and Testing
	if (p_enable)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
}

void GLRenderer::SetDepthTest(bool p_enable)
{
	// Enable/Disable testking only
	glDepthMask(BoolToGL(p_enable));
}

void GLRenderer::SetDepthAlgorythm(ComparaisonAlgorithm p_algorythm)
{
	glDepthFunc(ComparaisonToGL(p_algorythm));
}

void GLRenderer::EnableBlending(bool p_enable)
{
	if (p_enable)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
}

void GLRenderer::SetBlendingAlgorythm(BlendingAlgorithm p_source, BlendingAlgorithm p_dest)
{
	glBlendFunc(BlendingToGL(p_source), BlendingToGL(p_dest));
}

void GLRenderer::SetColorMask(bool p_red, bool p_green, bool p_blue, bool p_alpha)
{
	glColorMask(BoolToGL(p_red), BoolToGL(p_green), BoolToGL(p_blue), BoolToGL(p_alpha));
}

void GLRenderer::Draw(VertexArrayPtr p_vertexArray, size_t p_indicesCount, unsigned int p_indicesOffset, Primitives p_primitives)
{
	auto indices = p_vertexArray->GetIndexBuffer();
	if (indices == nullptr)
		glDrawArrays(PrimitiveToGL(p_primitives), 0, p_vertexArray->GetVertexBuffers()[0]->GetVertexNumber());
	else
	{
		if (p_indicesCount == 0)
			p_indicesCount = indices->GetSize();
		glDrawElements(PrimitiveToGL(p_primitives), p_indicesCount, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * p_indicesOffset));
	}
}

void GLRenderer::DrawArray(VertexBufferPtr p_vertexArray, Primitives p_primitives)
{
	glDrawArrays(PrimitiveToGL(p_primitives), 0, p_vertexArray->GetVertexNumber());
}

void GLRenderer::DrawInstanced(VertexArrayPtr p_vertexArray, size_t p_instanceCount, Primitives p_primitives)
{
	auto indices = p_vertexArray->GetIndexBuffer();
	if (indices == nullptr)
		glDrawArraysInstanced(PrimitiveToGL(p_primitives), 0, p_vertexArray->GetVertexBuffers()[0]->GetVertexNumber(), p_instanceCount);
	else
		glDrawElementsInstanced(PrimitiveToGL(p_primitives), indices->GetSize(), GL_UNSIGNED_INT, 0, p_instanceCount);
}

void GLRenderer::EnableCullFace(bool p_enable)
{
	if (p_enable)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
}

void GLRenderer::SetCullFaceMode(CullFaceMode p_mode)
{
	glCullFace(CullFaceToGL(p_mode));
}

void GLRenderer::BlitFrameBuffers(FramebuffferPtr p_readBuffer, FramebuffferPtr p_drawBuffer, FramebufferMask p_mask)
{
	PR_ASSERT(p_readBuffer != nullptr, "Read buffer is nullptr");

	auto& readSettings = p_readBuffer->GetSettings();
	const int readWidth = readSettings.globalWidth;
	const int readHeigth = readSettings.globalHeight;

	int drawWidth = readWidth;
	int drawHeigth = readHeigth;
	if(p_drawBuffer)
	{
		drawWidth = p_drawBuffer->GetSettings().globalWidth;
		drawHeigth = p_drawBuffer->GetSettings().globalHeight;;
	}
	glBindFramebuffer(GL_READ_FRAMEBUFFER, p_readBuffer ? p_readBuffer->GetID() : 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, p_drawBuffer ? p_drawBuffer->GetID() : 0);
	glBlitFramebuffer(0, 0, readWidth, readHeigth, 0, 0, drawWidth, drawHeigth, FramebufferMaskToGL(p_mask), GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void* GLRenderer::ReadFrontBuffer(size_t& p_outWidth, size_t& p_outHeight)
{
	glReadBuffer(GL_FRONT);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	p_outWidth = PrCore::Windowing::Window::GetMainWindow().GetWidth();
	p_outHeight = PrCore::Windowing::Window::GetMainWindow().GetHeight();

	byte* buffer = new byte[p_outWidth * p_outHeight * 3];
	glReadPixels(0, 0, p_outWidth, p_outHeight, GL_RGB, GL_UNSIGNED_BYTE, buffer);

	return buffer;
}
