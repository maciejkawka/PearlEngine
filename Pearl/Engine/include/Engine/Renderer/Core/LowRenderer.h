#pragma once 

#include "Renderer/Core/RendererAPI.h"
#include "Renderer/Core/RenderCommand.h"
#include "Renderer/Buffers/Framebuffer.h"

namespace PrRenderer {

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
		static void Draw(VertexArrayPtr p_vertexArray, size_t p_indicesCount = 0, unsigned int p_indicesOffset = 0, Primitives p_primitives = Primitives::Triangles);
		static void DrawArray(VertexBufferPtr p_vertexArray, Primitives p_primitives = Primitives::Triangles);
		static void DrawInstanced(VertexArrayPtr p_vertexArray, size_t p_instanceCount, Primitives p_primitives = Primitives::Triangles);

		//Other
		static void EnableCullFace(bool p_enable);
		static void SetCullFaceMode(CullFaceMode p_mode);

		//Utils
		static void BlitFrameBuffers(FramebuffferPtr p_readBuffer, FramebuffferPtr p_drawBuffer = nullptr, FramebufferMask p_mask = FramebufferMask::ColorBufferBit);
		static void* ReadFrontBuffer(size_t& p_outWidthm, size_t& p_outHeight);

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

		REGISTER_RENDER_COMMAND(Draw, VertexArrayPtr, size_t, unsigned int, Primitives);
		REGISTER_RENDER_COMMAND(DrawArray, VertexBufferPtr, Primitives);
		REGISTER_RENDER_COMMAND(DrawInstanced, VertexArrayPtr, size_t, Primitives);

		REGISTER_RENDER_COMMAND(EnableCullFace, bool);
		REGISTER_RENDER_COMMAND(SetCullFaceMode, CullFaceMode);

		REGISTER_RENDER_COMMAND(BlitFrameBuffers, FramebuffferPtr, FramebuffferPtr, FramebufferMask);

	private:
		inline static RendererAPI* m_rendererAPI = nullptr;
	};
}