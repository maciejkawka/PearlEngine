#pragma once
#include<memory>

namespace PrRenderer {

	namespace Buffers {
	class VertexBuffer;
	class IndexBuffer;
	class VertexArray;
	class Framebufffer;
}

	namespace Resources
	{
		class Mesh;


		typedef std::shared_ptr<Resources::Mesh> MeshPtr;
	}

	typedef unsigned int RendererID;
	typedef std::shared_ptr<Buffers::VertexBuffer> VertexBufferPtr;
	typedef std::shared_ptr<Buffers::IndexBuffer> IndexBufferPtr;
	typedef std::shared_ptr<Buffers::VertexArray> VertexArrayPtr;
	typedef std::shared_ptr<Buffers::Framebufffer> FramebuffferPtr;
}