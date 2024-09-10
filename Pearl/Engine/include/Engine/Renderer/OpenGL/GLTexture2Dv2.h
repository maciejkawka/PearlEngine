#pragma once

#include "Renderer/Resources/Texture2Dv2.h"

namespace PrRenderer::OpenGL {

	class GLTexture2Dv2 : public Resources::Texture2Dv2 {
	public:
		GLTexture2Dv2();
		GLTexture2Dv2(RendererID p_id, size_t p_width, size_t p_height, Resources::TextureFormat p_format);
		~GLTexture2Dv2() override;

		void Bind(unsigned int p_slot = 0) override;
		void Unbind(unsigned int p_slot = 0) override;
		void GenerateMipMaps() override;

		void Apply() override;

		size_t GetByteSize() const override;

	protected:
		void CalculateSize() override;
	};
}