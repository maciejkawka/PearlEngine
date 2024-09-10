#pragma once

#include"Renderer/Resources/Cubemapv2.h"

namespace PrRenderer::OpenGL {

	class GLCubemapv2 : public Resources::Cubemapv2 {
	public:
		GLCubemapv2();
		GLCubemapv2(RendererID p_id, size_t p_width, size_t p_height, Resources::TextureFormat p_format);
		~GLCubemapv2() override;

		void Bind(unsigned int p_slot = 0) override;
		void Unbind(unsigned int p_slot = 0) override;
		void GenerateMipMaps() override;

		void Apply() override;

		size_t GetByteSize() const override;

	protected:
		void CalculateSize() override;
	};
}