#pragma once

#include "Renderer/Resources/Texture2D.h"

#include<vector>

namespace PrRenderer::Resources {

	class Cubemap : public Texture2D {
	public:
		Cubemap() :
			Texture2D(),
			m_rawDataArray(nullptr),
			m_wrapR(TextureWrapMode::Clamp)
		{}

		Cubemap(RendererID p_id, size_t p_width, size_t p_height, Resources::TextureFormat p_format):
			Texture2D(),
			m_rawDataArray(nullptr),
			m_wrapR(TextureWrapMode::Clamp)
		{}

		// Texture factories
		static std::shared_ptr<Cubemap> Create();
		static std::shared_ptr<Cubemap> Create(RendererID p_id, size_t p_width, size_t p_height, Resources::TextureFormat p_format);
		static std::shared_ptr<Cubemap> CreateUnitTex(const Core::Color& p_unitColor);

		virtual void SetWrapModeR(TextureWrapMode p_wrapR) { m_wrapR = p_wrapR; }
		TextureWrapMode GetWrapModeR() const { return m_wrapR; }

		void SetData(void* p_data) override;

	protected:
		void** m_rawDataArray;
		TextureWrapMode m_wrapR;
	};

	REGISTRER_RESOURCE_HANDLE(Cubemap);
	using Cubemapv2Ptr = std::shared_ptr<Cubemap>;
}