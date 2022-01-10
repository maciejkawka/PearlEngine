#pragma once
#include"Renderer/Core/Defines.h"
#include"Renderer/Buffers/BufferLayout.h"

namespace PrRenderer::Buffers{

	class VertexBuffer {
	public:
		virtual ~VertexBuffer() {}

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void SetData(void* p_data, size_t p_size) = 0;

		virtual void SetBufferLayout(const BufferLayout& p_bufferLayout) = 0;
		inline virtual const BufferLayout GetBufferLayout() const { return m_bufferLayout; }

		inline size_t GetSize() { return m_size; }
		inline RendererID GetID() { return m_bufferID; }

	protected:
		RendererID m_bufferID;
		BufferLayout m_bufferLayout;
		size_t m_size;
	};
}