#include"Core/Common/pearl_pch.h"

#include"Renderer/OpenGL/GLVertexBuffer.h"

#include"glad/glad.h"

using namespace PrRenderer::OpenGL;

PrRenderer::OpenGL::GLVertexBuffer::GLVertexBuffer()
{
	glGenBuffers(1, &m_bufferID);
}

GLVertexBuffer::GLVertexBuffer(void* p_data, size_t p_size)
{
	glGenBuffers(1, &m_bufferID);
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID);
	glBufferData(GL_ARRAY_BUFFER, p_size, p_data, GL_STATIC_DRAW);
}

GLVertexBuffer::~GLVertexBuffer()
{
	glDeleteBuffers(1, &m_bufferID);
}

void GLVertexBuffer::Bind()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID);
}

void GLVertexBuffer::Unbind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLVertexBuffer::SetData(void* p_data, size_t p_size)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID);
	glBufferData(GL_ARRAY_BUFFER, p_size, p_data, GL_STATIC_DRAW);
}
