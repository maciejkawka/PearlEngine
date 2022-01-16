#pragma once
#include"Renderer/Core/Defines.h"
#include<vector>

namespace PrRenderer::Buffers {

	class VertexArray {
	public:
		virtual ~VertexArray() {}

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void SetVertexBuffer(const VertexBufferPtr p_vertexBuffer) = 0;
		virtual void SetIndexBuffer(const IndexBufferPtr p_indexBuffer) = 0;

		virtual const std::vector<VertexBufferPtr>& GetVertexBuffers() { return m_vertexBuffers; }
		virtual const IndexBufferPtr GetIndexBuffer() { return m_indexBuffer; }

		inline RendererID GetID() { return m_bufferID; }

	protected:
		RendererID m_bufferID;
		size_t m_vertexBufferIndex;
		std::vector<VertexBufferPtr> m_vertexBuffers;
		IndexBufferPtr m_indexBuffer;

	};
}