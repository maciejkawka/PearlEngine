#pragma once
#include<memory>

namespace PrRenderer {

	class VertexBuffer;
	class IndexBuffer;

	typedef unsigned int RendererID;
	typedef std::shared_ptr<VertexBuffer> VertexBufferPtr;
	typedef std::shared_ptr<IndexBuffer> IndexBufferPtr;
}