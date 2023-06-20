#pragma once

#include"Renderer/Core/RendererAPI.h"

namespace PrRenderer::OpenGL {
	
	class GLRenderer : public Core::RendererAPI {
	public:
		GLRenderer();
		~GLRenderer() override = default;

		void Clear(Core::ClearFlag p_flag) override;
		void ClearColor(float p_r, float p_g, float p_b, float p_a = 1.0f) override;
		void ClearColor(Core::Color& p_color) override;

		void SetViewport(int p_width, int p_height, int p_x = 0.0f, int p_y = 0.0f) override;

		void EnableDepth(bool p_enable) override;
		void SetDepthTest(bool p_enable) override;
		void SetDepthAlgorythm(Core::ComparaisonAlgorithm p_algorythm) override;

		void EnableBlending(bool p_enable) override;
		void SetBlendingAlgorythm(Core::BlendingAlgorithm p_source, Core::BlendingAlgorithm p_dest) override;

		void Draw(Buffers::VertexArrayPtr p_vertexArray, Core::Primitives p_primitives = Core::Primitives::Triangles) override;
		void DrawArray(Buffers::VertexBufferPtr p_vertexArray, Core::Primitives p_primitives = Core::Primitives::Triangles) override;

		void EnableCullFace(bool p_enable) override;
		void SetCullFaceMode(Core::CullFaceMode p_mode) override;
	};
}