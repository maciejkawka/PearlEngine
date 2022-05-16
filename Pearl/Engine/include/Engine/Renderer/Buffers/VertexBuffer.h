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

		inline void SetBufferLayout(const BufferLayout& p_bufferLayout) { m_bufferLayout = p_bufferLayout; }
		inline virtual const BufferLayout GetBufferLayout() const { return m_bufferLayout; }

		inline size_t GetVertexNumber() { return m_size/ m_bufferLayout.GetFloatStride() / 4; }
		inline size_t GetSize() { return m_size; }
		inline RendererID GetID() { return m_bufferID; }

		static VertexBufferPtr Create();
		static VertexBufferPtr Create(void* p_data, size_t p_size);
	protected:
		RendererID m_bufferID;
		BufferLayout m_bufferLayout;
		size_t m_size;
	};
}