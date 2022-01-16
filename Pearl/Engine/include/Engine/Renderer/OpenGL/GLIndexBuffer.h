#pragma once
#include"Renderer/Buffers/IndexBuffer.h"

namespace PrRenderer::OpenGL {

	class GLIndexBuffer : public Buffers::IndexBuffer{
	public:
		GLIndexBuffer();
		GLIndexBuffer(uint32_t* p_indices, uint32_t p_size);

		~GLIndexBuffer();

		void Bind() override;
		void Unbind() override;

		void SetIndeces(void* p_indeces, size_t p_size) override;
	};
}