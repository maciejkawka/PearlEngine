#pragma once
#include"Core/Math/Math.h"
#include"Renderer/Core/Color.h"

#include"Renderer/Buffers/VertexArray.h"
#include"Renderer/Buffers/Framebuffer.h"

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

	enum class BlendingAlgorithm
	{
		Zero,
		One,
		SourceColor,
		OneMinusSorceColor,
		DestinationColor,
		OneMinusDestinationColor,
		SourceAlpha,
		OneMinusSourceAlpha,
		DestinationAlpha,
		OneMinusDestinationAlpha
	};

	enum class CullFaceMode
	{
		Back,
		Front,
		BackAndFront
	};

	class RendererAPI {
	public:
		virtual ~RendererAPI() = default;

		virtual void Clear(ClearFlag p_flag) = 0;
		virtual void ClearColor(float p_r, float p_g, float p_b, float p_a = 1.0f) = 0;
		virtual void ClearColor(Color& p_color) = 0;

		virtual void SetViewport(int p_width, int p_height, int p_x = 0, int p_y = 0) = 0;
		virtual void SetViewportArray(int p_first, int p_count, float* p_viewportData) = 0;
		virtual void GetViewport(int& p_width, int& p_height, int& p_x, int& p_y) = 0;

		virtual void EnableDepth(bool p_enable) = 0;
		virtual void SetDepthTest(bool p_enable) = 0;
		virtual void SetDepthAlgorythm(ComparaisonAlgorithm p_algorythm) = 0;

		virtual void EnableBlending(bool p_enable) = 0;
		virtual void SetBlendingAlgorythm(BlendingAlgorithm p_source, BlendingAlgorithm p_dest) = 0;

		virtual void SetColorMask(bool p_red, bool p_green, bool p_blue, bool p_alpha) = 0;

		virtual void Draw(Buffers::VertexArrayPtr p_vertexArray, Primitives p_primitives = Primitives::Triangles) = 0;
		virtual void DrawArray(Buffers::VertexBufferPtr p_vertexArray, Primitives p_primitives = Primitives::Triangles) = 0;
		virtual void DrawInstanced(Buffers::VertexArrayPtr p_vertexArray, size_t p_instanceCount, Primitives p_primitives = Primitives::Triangles) = 0;
		virtual void EnableCullFace(bool p_enable) = 0;
		virtual void SetCullFaceMode(CullFaceMode p_mode) = 0;

		virtual void BlitFrameBuffers(Buffers::FramebuffferPtr p_readBuffer, Buffers::FramebuffferPtr p_drawBuffer, Buffers::FramebufferMask p_mask) = 0;

		inline static GraphicsAPI GetGraphicsAPI() { return m_graphicsAPI; }

	protected:
		inline static GraphicsAPI m_graphicsAPI = GraphicsAPI::None;
	};
}
