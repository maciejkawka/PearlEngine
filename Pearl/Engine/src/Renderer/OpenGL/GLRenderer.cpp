#include"Core/Common/pearl_pch.h"

#include "Renderer/OpenGL/GLRenderer.h"
#include"Renderer/OpenGL/GLUtils.h"

#include"glad/glad.h"

using namespace PrRenderer::OpenGL;

GLRenderer::GLRenderer()
{
	RendererAPI::m_graphicsAPI = Core::GraphicsAPI::OpenGL;
}

void GLRenderer::Clear(Core::ClearFlag p_flag)
{
	int mask = 0;

	if (p_flag & Core::ClearFlag::ColorBuffer)
		mask |= GL_COLOR_BUFFER_BIT;
	if (p_flag & Core::ClearFlag::DepthBuffer)
		mask |= GL_DEPTH_BUFFER_BIT;
	if (p_flag & Core::ClearFlag::StencilBuffer)
		mask |= GL_STENCIL_BUFFER_BIT;

	glClear(mask);
}

void GLRenderer::ClearColor(float p_r, float p_g, float p_b, float p_a)
{
	glClearColor(p_r, p_g, p_b, p_a);
}

void GLRenderer::ClearColor(Core::Color& p_color)
{
	glClearColor(p_color.r, p_color.g, p_color.b, p_color.a);
}

void GLRenderer::SetViewport(int p_width, int p_height, int p_x, int p_y)
{
	glViewport(p_x, p_y, p_width, p_height);
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

void GLRenderer::SetDepthAlgorythm(Core::ComparaisonAlgorithm p_algorythm)
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

void GLRenderer::SetBlendingAlgorythm(Core::BlendingAlgorithm p_source, Core::BlendingAlgorithm p_dest)
{
	glBlendFunc(BlendingToGL(p_source), BlendingToGL(p_dest));
}

void GLRenderer::Draw(Buffers::VertexArrayPtr p_vertexArray, Core::Primitives p_primitives)
{
	auto indices = p_vertexArray->GetIndexBuffer();
	if (indices == nullptr)
		glDrawArrays(PrimitiveToGL(p_primitives), 0, p_vertexArray->GetVertexBuffers()[0]->GetVertexNumber());
	else
		glDrawElements(PrimitiveToGL(p_primitives), indices->GetSize(), GL_UNSIGNED_INT, 0);
}

void GLRenderer::DrawArray(Buffers::VertexBufferPtr p_vertexArray, Core::Primitives p_primitives)
{
	glDrawArrays(PrimitiveToGL(p_primitives), 0, p_vertexArray->GetVertexNumber());
}

void GLRenderer::EnableCullFace(bool p_enable)
{
	if (p_enable)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
}

void GLRenderer::SetCullFaceMode(Core::CullFaceMode p_mode)
{
	glCullFace(CullFaceToGL(p_mode));
}
