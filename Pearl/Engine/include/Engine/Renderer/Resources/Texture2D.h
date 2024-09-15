#pragma once

#include "Renderer/Resources/Texture.h"

namespace PrRenderer::Resources {

	class Texture2D : public Texture {
	public:
		// Texture factories
		static std::shared_ptr<Texture2D> Create();
		static std::shared_ptr<Texture2D> CreateUnitTex(const Core::Color& p_unitColor);
	};

	REGISTRER_RESOURCE_HANDLE(Texture2D);
	using Texture2Dv2Ptr = std::shared_ptr<Texture2D>;
}
