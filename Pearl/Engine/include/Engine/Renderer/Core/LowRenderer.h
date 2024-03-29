#pragma once 

#include"Renderer/Core/RendererAPI.h"

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
		//static void EnableBlending();
		//static void SetBlendingAlgorythm();

		//Color Masks
		// static void SetColorMask()
		// static void SetColorMask() 
		
		//Draw
		static void Draw(Buffers::VertexArrayPtr p_vertexArray, Core::Primitives p_primitives = Core::Primitives::Triangles);
		static void DrawArray(Buffers::VertexBufferPtr p_vertexArray, Core::Primitives p_primitives = Core::Primitives::Triangles);

	private:
		inline static RendererAPI* m_rendererAPI = nullptr;

	};
}