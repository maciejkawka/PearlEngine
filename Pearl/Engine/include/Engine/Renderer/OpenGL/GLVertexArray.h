#pragma once

#include"Renderer/Buffers/VertexArray.h"

namespace PrRenderer::OpenGL {

	class GLVertexArray : public Buffers::VertexArray {
	public:
		GLVertexArray();
		~GLVertexArray();

		void Bind() override;
		void Unbind() override;

		void SetVertexBuffer(const VertexBufferPtr p_vertexBuffer) override;
		void SetIndexBuffer(const IndexBufferPtr p_indexBuffer) override;
	};
}