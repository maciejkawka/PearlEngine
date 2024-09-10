#pragma once

#include "Renderer/Resources/Texture2Dv2.h"

#include<vector>

namespace PrRenderer::Resources {

	class Cubemapv2 : public Texture2Dv2 {
	public:
		Cubemapv2() :
			Texture2Dv2(),
			m_rawDataArray(nullptr),
			m_wrapR(TextureWrapMode::Clamp)
		{}

		// Texture factories
		static std::shared_ptr<Cubemapv2> Create();
		static std::shared_ptr<Cubemapv2> CreateUnitTex(const Core::Color& p_unitColor);

		virtual void SetWrapModeR(TextureWrapMode p_wrapR) { m_wrapR = p_wrapR; }
		TextureWrapMode GetWrapModeR() const { return m_wrapR; }

		void SetData(void* p_data) override;

	protected:
		void** m_rawDataArray;
		TextureWrapMode m_wrapR;
	};
	using Cubemapv2Ptr = std::shared_ptr<Cubemapv2>;
}