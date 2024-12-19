#pragma once 

#include"Renderer/Core/RendererAPI.h"
#include"Renderer/Core/RenderCommand.h"
#include"Renderer/Buffers/Framebuffer.h"

namespace PrRenderer::Core {

	class LowRenderer {
	public:
		LowRenderer() = delete;
		LowRenderer(LowRenderer&) = delete;
		LowRenderer(LowRenderer&&) = delete;

		static void Init(GraphicsAPI p_API);
		static void Terminate();

		//Clear
		static void Clear(ClearFlag p_flag);
		static void ClearColor(float p_r, float p_g, float p_b, float p_a = 1.0f);
		static void ClearColor(Color& p_color);

		static void SetViewport(int p_width, int p_height, int p_x = 0, int p_y = 0);
		static void SetViewportArray(int p_first, int p_count, float* p_viewportData);
		static void GetViewport(int& p_width, int& p_height, int& p_x, int& p_y);

		//Depth
		static void EnableDepth(bool p_enable);
		static void SetDepthTest(bool p_enable);
		static void SetDepthAlgorythm(ComparaisonAlgorithm p_algorythm);
	
		//Stenicl
		//static void SetStencilAlgorythm();
		//static void SetStencilTest();
		//static void SetStencilOperation();
		//static void EnableStencil();
		
		//Culling
		//static void SetCullFace();

		//Blending
		static void EnableBlending(bool p_enable);
		static void SetBlendingAlgorythm(BlendingAlgorithm p_source, BlendingAlgorithm p_dest);

		//Color Masks
		static void SetColorMask(bool p_red, bool p_green, bool p_blue, bool p_alpha);
		// static void SetColorMask() 
		
		//Draw
		static void Draw(Buffers::VertexArrayPtr p_vertexArray, size_t p_indicesCount = 0, unsigned int p_indicesOffset = 0, Core::Primitives p_primitives = Core::Primitives::Triangles);
		static void DrawArray(Buffers::VertexBufferPtr p_vertexArray, Core::Primitives p_primitives = Core::Primitives::Triangles);
		static void DrawInstanced(Buffers::VertexArrayPtr p_vertexArray, size_t p_instanceCount, Primitives p_primitives = Primitives::Triangles);

		//Other
		static void EnableCullFace(bool p_enable);
		static void SetCullFaceMode(CullFaceMode p_mode);

		//Utils
		static void BlitFrameBuffers(Buffers::FramebuffferPtr p_readBuffer, Buffers::FramebuffferPtr p_drawBuffer = nullptr, Buffers::FramebufferMask p_mask = Buffers::FramebufferMask::ColorBufferBit);

		//Commands
		REGISTER_RENDER_COMMAND(Clear, ClearFlag);
		REGISTER_NAMED_RENDER_COMMAND(ClearColorFloat, ClearColor, float, float, float, float);
		REGISTER_NAMED_RENDER_COMMAND(ClearColorColor, ClearColor, Color);

		REGISTER_RENDER_COMMAND(SetViewport, int, int, int, int);
		REGISTER_RENDER_COMMAND(SetViewportArray, int, int, float*);

		REGISTER_RENDER_COMMAND(EnableDepth, bool);
		REGISTER_RENDER_COMMAND(SetDepthTest, bool);
		REGISTER_RENDER_COMMAND(SetDepthAlgorythm, ComparaisonAlgorithm);

		REGISTER_RENDER_COMMAND(EnableBlending, bool);
		REGISTER_RENDER_COMMAND(SetBlendingAlgorythm, BlendingAlgorithm, BlendingAlgorithm);

		REGISTER_RENDER_COMMAND(Draw, Buffers::VertexArrayPtr, size_t, unsigned int, Core::Primitives);
		REGISTER_RENDER_COMMAND(DrawArray, Buffers::VertexBufferPtr, Core::Primitives);
		REGISTER_RENDER_COMMAND(DrawInstanced, Buffers::VertexArrayPtr, size_t, Core::Primitives);

		REGISTER_RENDER_COMMAND(EnableCullFace, bool);
		REGISTER_RENDER_COMMAND(SetCullFaceMode, CullFaceMode);

		REGISTER_RENDER_COMMAND(BlitFrameBuffers, Buffers::FramebuffferPtr, Buffers::FramebuffferPtr, Buffers::FramebufferMask);

	private:
		inline static RendererAPI* m_rendererAPI = nullptr;
	};
}