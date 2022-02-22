#pragma once
#include<memory>

namespace PrRenderer {

	namespace Buffers {
	class VertexBuffer;
	class IndexBuffer;
	class VertexArray;
}

	typedef unsigned int RendererID;
	typedef std::shared_ptr<Buffers::VertexBuffer> VertexBufferPtr;
	typedef std::shared_ptr<Buffers::IndexBuffer> IndexBufferPtr;
	typedef std::shared_ptr<Buffers::VertexArray> VertexArrayPtr;
}