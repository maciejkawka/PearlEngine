#include"Core/Common/pearl_pch.h"
#include"Renderer/OpenGL/GLIndexBuffer.h"

#include"glad/glad.h"

using namespace PrRenderer::OpenGL;

GLIndexBuffer::GLIndexBuffer()
{
	glGenBuffers(1, &m_bufferID);
}

GLIndexBuffer::GLIndexBuffer(uint32_t* p_indices, uint32_t p_size)
{
	glGenBuffers(1, &m_bufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, p_size * sizeof(uint32_t), p_indices, GL_STATIC_DRAW);
	m_size = p_size;
}

GLIndexBuffer::~GLIndexBuffer()
{
	glDeleteBuffers(1, &m_bufferID);
}

void GLIndexBuffer::Bind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferID);
}

void GLIndexBuffer::Unbind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLIndexBuffer::SetIndeces(void* p_indeces, size_t p_size)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, p_size * sizeof(uint32_t), p_indeces, GL_STATIC_DRAW);
	m_size = p_size;
}


