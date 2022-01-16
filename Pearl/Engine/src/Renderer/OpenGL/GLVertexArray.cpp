#include"Core/Common/pearl_pch.h"
#include"Renderer/OpenGL/GLVertexArray.h"

#include"Renderer/Buffers/VertexBuffer.h"
#include"Renderer/Buffers/IndexBuffer.h"
#include"glad/glad.h"

using namespace PrRenderer::OpenGL;

GLVertexArray::GLVertexArray()
{
	m_vertexBufferIndex = 0;
	glGenVertexArrays(1, &m_bufferID);
}

GLVertexArray::~GLVertexArray()
{
	glDeleteBuffers(1, &m_bufferID);
}

void GLVertexArray::Bind()
{
	glBindVertexArray(m_bufferID);
}

void GLVertexArray::Unbind()
{
	glBindVertexArray(0);
}

void GLVertexArray::SetVertexBuffer(const VertexBufferPtr p_vertexBuffer)
{
	glBindVertexArray(m_bufferID);
	p_vertexBuffer->Bind();

	auto& layout = p_vertexBuffer->GetBufferLayout();
	for (auto& element : layout.GetElements())
	{
		switch (element.type)
		{
		case Buffers::ShaderDataType::Float:
		case Buffers::ShaderDataType::Float2:
		case Buffers::ShaderDataType::Float3:
		case Buffers::ShaderDataType::Float4:
		{
			glVertexAttribPointer(m_vertexBufferIndex,
				element.GetTypeSize(),
				GL_FLOAT,
				element.normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.offset);
			glEnableVertexAttribArray(m_vertexBufferIndex);
			m_vertexBufferIndex++;
			break;
		}

		case Buffers::ShaderDataType::Int:
		case Buffers::ShaderDataType::Int2:
		case Buffers::ShaderDataType::Int3:
		case Buffers::ShaderDataType::Int4:
		{
			glVertexAttribIPointer(m_vertexBufferIndex,
				element.GetTypeSize(),
				GL_INT,
				layout.GetStride(),
				(const void*)element.offset);
			glEnableVertexAttribArray(m_vertexBufferIndex);
			m_vertexBufferIndex++;
			break;
		}

		case Buffers::ShaderDataType::Bool:
		{
			glVertexAttribIPointer(m_vertexBufferIndex,
				element.GetTypeSize(),
				GL_BYTE,
				layout.GetStride(),
				(const void*)element.offset);
			glEnableVertexAttribArray(m_vertexBufferIndex);
			m_vertexBufferIndex++;
			break;
		}

		case Buffers::ShaderDataType::Mat3:
		case Buffers::ShaderDataType::Mat4:
		{
			for (uint32_t column = 0; column < element.size; column++)
			{
				glVertexAttribPointer(m_vertexBufferIndex,
					element.size,
					GL_FLOAT,
					element.normalized ? GL_TRUE : GL_FALSE,
					layout.GetStride(),
					(const void*)(element.offset + element.size * sizeof(float) * column));
				glEnableVertexAttribArray(m_vertexBufferIndex);
				m_vertexBufferIndex++;

			}
			break;
		}

		default:
			PRLOG_ERROR("Renderer: ShaderDataType Unknown! bufferID: {0} element: {1}", m_bufferID, element.name);
			p_vertexBuffer->Unbind();
			return;
		}
	}

	m_vertexBuffers.push_back(p_vertexBuffer);
	glBindVertexArray(0);
}

void GLVertexArray::SetIndexBuffer(const IndexBufferPtr p_indexBuffer)
{
	glBindVertexArray(m_bufferID);
	m_indexBuffer = p_indexBuffer;
	m_indexBuffer->Bind();
	glBindVertexArray(0);
}




