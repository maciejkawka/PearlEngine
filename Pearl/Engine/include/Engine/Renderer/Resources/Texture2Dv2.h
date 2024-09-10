#pragma once

#include "Renderer/Resources/Texturev2.h"

namespace PrRenderer::Resources {

	class Texture2Dv2 : public Texturev2 {
	public:
		// Texture factories
		static std::shared_ptr<Texture2Dv2> Create();
		static std::shared_ptr<Texture2Dv2> CreateUnitTex(const Core::Color& p_unitColor);
	};
	using Texture2Dv2Ptr = std::shared_ptr<Texture2Dv2>;
}
