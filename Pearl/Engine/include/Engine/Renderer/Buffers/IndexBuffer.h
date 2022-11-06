#pragma once
#include"Renderer/Core/Defines.h"

namespace PrRenderer::Buffers {

	class IndexBuffer;
	typedef std::shared_ptr<IndexBuffer> IndexBufferPtr;

	class IndexBuffer {
	public:
		IndexBuffer() = default;
		virtual ~IndexBuffer() {}

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void SetIndeces(void* p_indeces, size_t p_size) = 0;

		inline size_t GetSize() { return m_size; }
		inline RendererID GetID() { return m_bufferID; }

		static IndexBufferPtr Create();

	protected:
		RendererID m_bufferID;
		size_t m_size;
	};
}