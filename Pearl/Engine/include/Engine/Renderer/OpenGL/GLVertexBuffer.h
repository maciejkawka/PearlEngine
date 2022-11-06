#pragma once
#include"Renderer/Buffers/VertexBuffer.h"

namespace PrRenderer::OpenGL {

	class GLVertexBuffer : public Buffers::VertexBuffer {
	public:
		GLVertexBuffer();
		GLVertexBuffer(void* p_data, size_t p_size);

		~GLVertexBuffer() override;

		void Bind() override;
		void Unbind() override;

		void SetData(void* p_data, size_t p_size) override;
	};
}