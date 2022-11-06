#pragma once
#include"Core/Math/Math.h"
#include"Renderer/Core/Color.h"

#include"Renderer/Buffers/VertexArray.h"

namespace PrRenderer::Core {

	enum ClearFlag {
		ColorBuffer = 1,
		DepthBuffer = 2,
		StencilBuffer = 4
	};

	DEFINE_ENUM_FLAG_OPERATORS(ClearFlag);

	enum class GraphicsAPI {
		OpenGL,
		DirectX,
		None
	};

	enum class Primitives {
		Points,
		LineStrip,
		TriangleStrip,
		Triangles
	};

	enum class ComparaisonAlgorithm
	{
		Never,
		Less,
		Equal,
		LessEqual,
		Greater,
		NotEqual,
		GreaterEqual,
		Always
	};


	class RendererAPI {
	public:
		virtual ~RendererAPI() = default;

		virtual void Clear(ClearFlag p_flag) = 0;
		virtual void ClearColor(float p_r, float p_g, float p_b, float p_a = 1.0f) = 0;
		virtual void ClearColor(Color& p_color) = 0;

		virtual void SetViewport(int p_width, int p_height, int p_x = 0, int p_y = 0) = 0;

		virtual void EnableDepth(bool p_enable) = 0;
		virtual void SetDepthTest(bool p_enable) = 0;
		virtual void SetDepthAlgorythm(ComparaisonAlgorithm p_algorythm) = 0;

		virtual void Draw(Buffers::VertexArrayPtr p_vertexArray, Primitives p_primitives = Primitives::Triangles) = 0;
		virtual void DrawArray(Buffers::VertexBufferPtr p_vertexArray, Primitives p_primitives = Primitives::Triangles) = 0;

		inline static GraphicsAPI GetGraphicsAPI() { return m_graphicsAPI; }

	protected:
		inline static GraphicsAPI m_graphicsAPI = GraphicsAPI::None;
	};
}
