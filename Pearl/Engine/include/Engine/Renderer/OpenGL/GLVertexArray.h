#pragma once

#include"Renderer/Buffers/VertexArray.h"

namespace PrRenderer::OpenGL {

	class GLVertexArray : public Buffers::VertexArray {
	public:
		GLVertexArray();
		~GLVertexArray() override;

		void Bind() override;
		void Unbind() override;

		void SetVertexBuffer(const Buffers::VertexBufferPtr p_vertexBuffer) override;
		void SetIndexBuffer(const Buffers::IndexBufferPtr p_indexBuffer) override;
	};
}