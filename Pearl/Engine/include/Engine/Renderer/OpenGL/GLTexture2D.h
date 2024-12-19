#pragma once

#include "Renderer/Resources/Texture2D.h"

namespace PrRenderer::OpenGL {

	class GLTexture2D : public Resources::Texture2D {
	public:
		GLTexture2D();
		GLTexture2D(RendererID p_id, size_t p_width, size_t p_height, Resources::TextureFormat p_format);
		~GLTexture2D() override;

		void Bind(unsigned int p_slot = 0) override;
		void Unbind(unsigned int p_slot = 0) override;
		void GenerateMipMaps() override;

		void Apply() override;
		void ClearWithColor(const Core::Color& p_color) override;

		size_t GetByteSize() const override;

	protected:
		void CalculateSize() override;
	};
}