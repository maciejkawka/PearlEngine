#pragma once

#include"Renderer/Resources/Cubemap.h"

namespace PrRenderer::OpenGL {

	class GLCubemap : public Resources::Cubemap {
	public:
		GLCubemap();
		GLCubemap(RendererID p_id, size_t p_width, size_t p_height, Resources::TextureFormat p_format);
		~GLCubemap() override;

		void Bind(unsigned int p_slot = 0) override;
		void Unbind(unsigned int p_slot = 0) override;
		void GenerateMipMaps() override;

		void Apply() override;

		size_t GetByteSize() const override;

	protected:
		void CalculateSize() override;
	};
}