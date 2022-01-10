#pragma once
#include"Renderer/Core/Defines.h"
#include<vector>

namespace PrRenderer::Buffers {

	class VertexArray {
	public:
		virtual ~VertexArray() {}

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void SetVertecBuffer(const VertexBufferPtr p_vertexBuffer) = 0;
		virtual void SetIndexBuffer(const IndexBufferPtr p_indexBuffer) = 0;

		virtual void RemoveVertexBuffer(int p_index) = 0;

		virtual const std::vector<VertexBufferPtr>& GetVertexBuffers() = 0;
		virtual const IndexBufferPtr GetIndexBuffer() = 0;

		inline RendererID GetID() { return m_bufferID; }

	protected:
		RendererID m_bufferID;
		std::vector<VertexBufferPtr> m_vertexBuffers;
		IndexBufferPtr m_indexBuffer;

	};
}