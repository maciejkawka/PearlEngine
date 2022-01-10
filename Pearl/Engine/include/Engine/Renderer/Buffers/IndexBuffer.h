#pragma once
#include"Renderer/Core/Defines.h"

namespace PrRenderer::Buffers {

	class IndexBuffer {
	public:
		virtual ~IndexBuffer() {}

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void SetIndecies(void* p_indecies, size_t p_size) = 0;

		inline size_t GetSize() { return m_size; }
		inline RendererID GetID() { return m_bufferID; }

	protected:
		RendererID m_bufferID;
		size_t m_size;
	};
}